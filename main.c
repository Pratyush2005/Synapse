#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

void print_help() {
    printf("Usage: aiprocessmon [--top N] [--explain PID] [--help]\n");
    printf("  --top N       Show top N processes (default 10)\n");
    printf("  --explain PID Simple explanation of process\n");
    printf("  --help        Show this help\n");
}

void remove_trailing_whitespace(char *str) {
    char *pos;
    if ((pos = strchr(str, '\n')) != NULL) *pos = '\0';
    int len = strlen(str);
    while (len > 0 && str[len-1] == ' ') str[--len] = '\0';
}

void escape_quotes(char *str) {
    char temp[1024];
    int j = 0;
    for (int i = 0; str[i]; i++) {
        if (str[i] == '"') temp[j++] = '\\';
        temp[j++] = str[i];
    }
    temp[j] = '\0';
    strcpy(str, temp);
}

void handle_streamed_response(FILE *fp) {
    char buffer[4096];
    printf("\n🧠 Explanation:\n");
    
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

int main(int argc, char *argv[]) {
    int top_n = 10;
    int explain_pid = -1;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--help")) {
            print_help();
            return 0;
        } else if (!strcmp(argv[i], "--top") && i+1 < argc) {
            top_n = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--explain") && i+1 < argc) {
            explain_pid = atoi(argv[++i]);
        }
    }

    if (explain_pid > 0) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "ps -p %d -o command=", explain_pid);
        FILE *fp = popen(cmd, "r");
        if (!fp) {
            perror("ps failed");
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
            "\\\"content\\\": \\\"Give 1-sentence plain English explanation of processes\\\"},"
            "{\\\"role\\\": \\\"user\\\", "
            "\\\"content\\\": \\\"What is this? %s\\\"}],"
            "\\\"temperature\\\": 1,"
            "\\\"max_completion_tokens\\\": 1024,"
            "\\\"top_p\\\": 1,"
            "\\\"stream\\\": true"
            "}\"",
            getenv("GROQ_API_KEY"), process_info);

        fp = popen(curl_cmd, "r");
        if (!fp) {
            perror("curl failed");
            return 1;
        }
        
        handle_streamed_response(fp);
        pclose(fp);
        return 0;
    }

    printf("📋 Top %d Processes:\n\n", top_n);
    char ps_cmd[64];
    snprintf(ps_cmd, sizeof(ps_cmd), "ps aux | head -n %d", top_n + 1);
    system(ps_cmd);
    return 0;
}
