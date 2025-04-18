# 🧠 Synapse

**Synapse** is a minimalistic, AI-powered terminal tool for macOS that lets you monitor processes, inspect network connections, and get AI-powered explanations or troubleshooting suggestions. It combines Unix tools (`ps`, `lsof`) with Groq's LLM (`gemma2-9b-it`) to give real-time insights and advice.

---

## ⚙️ Features

- 📊 Display top N running processes using `ps`
- 🤖 Use Groq API to explain what a process does (PID-based)
- 🌐 View active network connections via `lsof`
- 🔧 Get AI-powered troubleshooting suggestions for system/network issues
- ⚡ Pure C performance — fast and lightweight
- 🔐 Minimal dependencies: `curl`, `ps`, `lsof`, `cJSON`

---

## 📦 Requirements

- macOS (tested on Apple Silicon)
- [Groq API Key](https://console.groq.com/)
- GCC or Clang compiler
- Internet connection (for AI features)

---

## 🔧 Installation

```bash
git clone https://github.com/Pratyush2005/Synapse.git
cd Synapse
gcc main.c cJSON.c -o synapse
```

---

## 🚀 Usage

```bash
./synapse --top 15              # Show top 15 processes
./synapse --explain <PID>       # AI explanation for a specific PID
./synapse --net                 # Show active network connections
./synapse --ai-help "wifi slow" # Get AI suggestions to fix your issue
./synapse --help                # Show help
```

---

## 🧠 Examples

```bash
$ ./synapse --explain 1208

🧠 Process Explanation:
This is the main Firefox browser process, responsible for rendering tabs and UI.

$ ./synapse --net

🌐 Active Network Connections:
PID      CONNECTION          PROCESS   
1234     127.0.0.1:3000      node      
5678     0.0.0.0:22          sshd      

$ ./synapse --ai-help "wifi disconnects frequently"

🔧 Recommended Actions:
Try restarting your DNS resolver:
sudo dscacheutil -flushcache; sudo killall -HUP mDNSResponder
```

---

## 🔐 Environment Setup

You **must** export your Groq API key:

```bash
export GROQ_API_KEY="your_groq_api_key"
```

---

## 🛠️ Developer Notes

- `main.c` — CLI logic, Groq API calls, process & network handling
- `handle_streamed_response()` handles real-time output from streaming API
- Uses `cJSON` for parsing responses
- Uses `curl` to call Groq’s `gemma2-9b-it` LLM

---


Made with 💻 by [Pratyush](https://github.com/Pratyush2005)
