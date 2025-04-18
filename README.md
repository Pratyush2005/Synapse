# 🧠 Synapse

**Synapse** is a minimalistic, AI-powered terminal tool for macOS that lets you monitor processes and understand what they do — instantly. It combines Unix commands like `ps` with the power of LLMs (via Groq) to provide real-time, plain-English explanations.

---

## ⚙️ Features

- 📊 Display top N running processes using `ps`
- 🤖 Use Groq API with the `gemma2-9b-it` model to explain what a process is doing
- ⚡ Pure C performance — fast and lightweight
- 🔐 No dependencies except `curl`, `ps`, and `cJSON`

---

## 📦 Requirements

- macOS (tested on Apple Silicon)
- [Groq API Key](https://console.groq.com/)
- GCC or Clang compiler
- Internet connection (for AI explanation)

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
./synapse --top 15           # Show top 15 processes
./synapse --explain <PID>    # AI explanation for specific PID
./synapse --help             # Show CLI usage
```

---

## 🧠 Example

```bash
$ ./synapse --explain 1208

🧠 Getting info for PID 1208...

Process info: /Applications/Firefox.app/Contents/MacOS/firefox

🧠 Explanation:
This is the Firefox web browser process, used for web browsing and developer tools.
```

---

## 🔐 Environment Setup

You **must** export your Groq API key:

```bash
export GROQ_API_KEY="your_groq_api_key"
```

---

## 🛠️ Developer Notes

- `main.c` — contains CLI handling, process extraction, AI integration
- Uses `curl` to call the Groq API
- Uses `cJSON` for parsing streaming JSON responses
- Minimal memory-safe helper utilities for escaping strings and cleaning output

---

## 📜 License

MIT License

---

Built with ❤️ by [Pratyush](https://github.com/Pratyush2005)
