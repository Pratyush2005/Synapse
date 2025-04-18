#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cJSON.h"

void print_help() {
    printf("Usage: Synapse [OPTIONS]\n");
    printf("  --top N          Show top N processes\n");
    printf("  --explain PID    Explain process with PID\n");
    printf("  --net            Show network connections\n");
    printf("  --ai-help QUERY  Get troubleshooting suggestions\n");
    printf("  --help           Show this help\n");
}

void remove_trailing_whitespace(char *str) {
    char *pos;
    if ((pos = strchr(str, '\n')) != NULL) *pos = '\0';
    int len = strlen(str);
    while (len > 0 && str[len-1] == ' ') str[--len] = '\0';
}

void escape_quotes(char *str) {
    char temp[32768];
    int j = 0;
    for (int i = 0; str[i] && j < sizeof(temp)-1; i++) {
        if (str[i] == '"' && j < sizeof(temp)-2) {
            temp[j++] = '\\';
        }
        if (j < sizeof(temp)-1) {
            temp[j++] = str[i];
        }
    }
    temp[j] = '\0';
    strcpy(str, temp);
}

void handle_streamed_response(FILE *fp, const char *prefix) {
    char buffer[4096];
    printf("\n%s", prefix);
    
    while (fgets(buffer, sizeof(buffer), fp)) {
        if (strncmp(buffer, "data: ", 6) == 0) {
            cJSON *root = cJSON_Parse(buffer + 6);
            if (root) {
                cJSON *choices = cJSON_GetObjectItem(root, "choices");
                if (cJSON_IsArray(choices)) {
                    cJSON *delta = cJSON_GetObjectItem(cJSON_GetArrayItem(choices, 0), "delta");
                    if (delta) {
                        cJSON *content = cJSON_GetObjectItem(delta, "content");
                        if (cJSON_IsString(content)) {
                            printf("%s", content->valuestring);
                            fflush(stdout);
                        }
                    }
                }
                cJSON_Delete(root);
            }
        }
    }
    printf("\n");
}

void show_network_processes() {
    printf("\n🌐 Active Network Connections:\n");
    printf("%-8s %-20s %-10s\n", "PID", "CONNECTION", "PROCESS");
    system("lsof -i -P -n | grep -E '(LISTEN|ESTABLISHED)' | awk '{printf \"%-8s %-20s %-10s\\n\", $2, $9, $1}' | head -n 20");
}

int main(int argc, char *argv[]) {
    int top_n = 10;
    int explain_pid = -1;
    int network_flag = 0;
    char *ai_query = NULL;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--help")) {
            print_help();
            return 0;
        } else if (!strcmp(argv[i], "--top") && i+1 < argc) {
            top_n = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--explain") && i+1 < argc) {
            explain_pid = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--net")) {
            network_flag = 1;
        } else if (!strcmp(argv[i], "--ai-help") && i+1 < argc) {
            ai_query = argv[++i];
        }
    }

    if (ai_query) {
        char ps_output[8192] = {0};
        char net_output[8192] = {0};
        size_t bytes_read;
        
        FILE *fp = popen("ps aux | head -n 50", "r");
        if (!fp) {
            perror("ps failed");
            return 1;
        }
        bytes_read = fread(ps_output, 1, sizeof(ps_output)-1, fp);
        ps_output[bytes_read] = '\0';
        pclose(fp);
        
        fp = popen("lsof -i -P -n | head -n 30", "r");
        if (!fp) {
            perror("lsof failed");
            return 1;
        }
        bytes_read = fread(net_output, 1, sizeof(net_output)-1, fp);
        net_output[bytes_read] = '\0';
        pclose(fp);

        char prompt[16384];
        snprintf(prompt, sizeof(prompt),
            "System State:\nPROCESSES:\n%.6000s\n\nNETWORK:\n%.6000s\n\nUser Issue: %s\n"
            "Provide specific terminal commands to resolve.",
            ps_output, net_output, ai_query);
        
        escape_quotes(prompt);

        if (!getenv("GROQ_API_KEY")) {
            fprintf(stderr, "GROQ_API_KEY environment variable not set\n");
            return 1;
        }

        char curl_cmd[32768];
        snprintf(curl_cmd, sizeof(curl_cmd),
            "curl -sS \"https://api.groq.com/openai/v1/chat/completions\" "
            "-H \"Content-Type: application/json\" "
            "-H \"Authorization: Bearer %s\" "
            "-d \"{"
            "\\\"model\\\": \\\"gemma2-9b-it\\\","
            "\\\"messages\\\": [{"
            "\\\"role\\\": \\\"system\\\", "
            "\\\"content\\\": \\\"You are a UNIX sysadmin assistant. Give exact commands to fix issues.\\\"},"
            "{\\\"role\\\": \\\"user\\\", "
            "\\\"content\\\": \\\"%s\\\"}],"
            "\\\"temperature\\\": 0.5,"
            "\\\"max_tokens\\\": 256,"
            "\\\"stream\\\": true"
            "}\"",
            getenv("GROQ_API_KEY"), prompt);

        fp = popen(curl_cmd, "r");
        if (!fp) {
            perror("API call failed");
            return 1;
        }
        
        handle_streamed_response(fp, "🔧 Recommended Actions:\n");
        pclose(fp);
        return 0;
    }

    if (explain_pid > 0) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "ps -p %d -o command=", explain_pid);
        FILE *fp = popen(cmd, "r");
        if (!fp) {
            perror("Process lookup failed");
            return 1;
        }

        char process_info[256] = {0};
        if (!fgets(process_info, sizeof(process_info), fp)) {
            fprintf(stderr, "PID %d not found\n", explain_pid);
            return 1;
        }
        pclose(fp);
        remove_trailing_whitespace(process_info);
        escape_quotes(process_info);

        if (!getenv("GROQ_API_KEY")) {
            fprintf(stderr, "GROQ_API_KEY not set\n");
            return 1;
        }

        char curl_cmd[2048];
        snprintf(curl_cmd, sizeof(curl_cmd),
            "curl -sS \"https://api.groq.com/openai/v1/chat/completions\" "
            "-H \"Content-Type: application/json\" "
            "-H \"Authorization: Bearer %s\" "
            "-d \"{"
            "\\\"model\\\": \\\"gemma2-9b-it\\\","
            "\\\"messages\\\": [{"
            "\\\"role\\\": \\\"system\\\", "
            "\\\"content\\\": \\\"Give 1-line process explanations\\\"},"
            "{\\\"role\\\": \\\"user\\\", "
            "\\\"content\\\": \\\"What is this? %s\\\"}],"
            "\\\"temperature\\\": 0.3,"
            "\\\"max_tokens\\\": 128,"
            "\\\"stream\\\": true"
            "}\"",
            getenv("GROQ_API_KEY"), process_info);

        fp = popen(curl_cmd, "r");
        if (!fp) {
            perror("API call failed");
            return 1;
        }
        
        handle_streamed_response(fp, "🧠 Process Explanation:\n");
        pclose(fp);
        return 0;
    }

    if (network_flag) {
        show_network_processes();
        return 0;
    }

    printf("📋 Top %d Processes:\n\n", top_n);
    char ps_cmd[64];
    snprintf(ps_cmd, sizeof(ps_cmd), "ps aux | head -n %d", top_n + 1);
    system(ps_cmd);
    
    return 0;
}
