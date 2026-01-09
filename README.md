# Theory of Computation – Compiler Project

##  Overview

This is a **Qt‑based C++ desktop application** that provides an interactive environment for exploring the core concepts of automata theory, lexical analysis, parsing, and semantic analysis. It is designed as a teaching tool for compiler construction courses, offering a **rich, dark‑mode UI** with smooth animations, micro‑interactions, and a polished look.

---

## Demo And Media
![Demo](https://github.com/Khorrrr/Theory-of-computation-project/releases/download/v1.0.0/demo-toc.gif)

##  Key Features

| Category | Functionality | Description |
|---|---|---|
| **Automaton Designer** | **Create New Automaton** | Dialog to create a DFA or NFA with a custom name. |
| | **Add / Delete / Move States** | Click‑to‑add states, drag to reposition, delete via context menu. |
| | **Add / Delete Transitions** | Click two states to create a transition; multiple transitions per symbol for NFA. |
| | **Convert NFA → DFA** | Automatic subset construction using `NFAtoDFA`. |
| | **Minimize DFA** | Hopcroft's algorithm via `DFAMinimizer`. |
| | **Regex → NFA** | Build an NFA from a regular expression using `RegexToNFA`. |
| | **Simulation Controls** | Play, step, stop, reset, speed slider with live visual trace. |
| **Lexical Analyzer** | **Tokenization UI** | Visualize token stream for a given source file. |
| | **Automaton‑Driven Lexer** | Uses the defined automaton to drive lexical analysis. |
| **Grammar & Parsing** | **Grammar Viewer** | Tree view of productions and parse trees (`ParseTreeWidget`). |
| | **Parser Generator** | Generates a LL(1) parsing table (`utils/Grammar/Parser`). |
| **Semantic Analyzer** | **Symbol Table** | Visual representation of scopes and symbols. |
| | **Code Generator** | Generates target code from the AST (`Semantic/CodeGenerator`). |
| **UI/UX** | **Dark Theme & Glass‑morphism** | Custom stylesheet with gradients, hover effects, and animated transitions. |
| | **Responsive Layout** | Dockable panels (Tools, Automaton List, Properties) that can be rearranged. |
| | **Micro‑animations** | Smooth toolbar button hover, dialog fade‑in, and state selection highlights. |

---

##  Architecture

```
src/
├─ main.cpp                     # Application entry point
├─ ui/                          # Qt widgets (MainWindow, Canvas, dialogs)
│   ├─ AutomatonCanvas.cpp/h    # Visual canvas for automata
│   ├─ Grammar/…                # Grammar visualisation widgets
│   ├─ LexicalAnalysis/…        # Lexer UI
│   ├─ Semantic/…               # Semantic analysis UI
│   └─ MainWindow.cpp/h         # Central window and toolbar
├─ models/                      # Core data structures
│   ├─ Automaton/…              # Automaton, State, Transition classes
│   ├─ Grammar/…                # Grammar, Production, ParseTree
│   ├─ LexicalAnalysis/…        # Token class
│   └─ Semantic/…               # AST, SymbolTable
└─ utils/                       # Algorithms & helpers
    ├─ Automaton/…              # NFA→DFA, DFA minimizer, Regex→NFA
    ├─ Grammar/Parser.cpp/h     # LL(1) parser generator
    ├─ LexicalAnalysis/…        # Lexer implementation
    └─ Semantic/…               # Code generation & analysis
```

*All UI components communicate with the model through signals/slots, ensuring a clean separation of concerns.*

---

##  Build & Run

### Prerequisites
- **Qt 6** (Core, Widgets, Gui) – install via the Qt installer.
- **CMake** ≥ 3.20
- A C++17 compatible compiler (MSVC on Windows).

### Steps
```bash
# Clone the repository (if not already present)
git clone https://github.com/Khorrrr/Theory-of-computation-project.git
cd tf-v3

# Build the project
Head to qt creator ide, create a new qt widget application using qmake
replace the src folder with the existing one

# Setup the .pro folder
Include the path for each folder inside the .pro file that way qt will see these files

# Build
Build the project and run it
```

---

## Usage Guide

1. **Launch the app** – the main window shows three dockable panels.
2. **Create an Automaton** – click the **New** button in the Automaton List panel, choose DFA or NFA.
3. **Edit** – use the toolbar to switch between *Select*, *Add State*, *Add Transition*, and *Delete* modes.
4. **Convert / Minimize** – press the **Convert NFA → DFA** or ** Minimize DFA** buttons.
5. **Regex → NFA** – click the ** From Regex** button and enter a regular expression.
6. **Simulation** – use the play/step/stop controls; adjust speed with the slider.
7. **Lexical Analysis** – load a source file via **File → Open**, view tokens in the Lexical Analysis tab.
8. **Parsing** – after defining a grammar, generate the parse table and visualise parse trees.
9. **Semantic Analysis**

---

## Improvments
-  Add a machine learning model, that uses a fine tuned model from Hugging Face to be able to make better Code Translation.
-  When a regex automaton is generated, let it be properly displayed not all messy
  


