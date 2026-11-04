with open("src/MainWindow.cpp", "r") as f:
    code = f.read()

target = "        QHeaderView::section {"
replacement = """        QFrame#ComplianceCard {
            background-color: #ffffff;
            border: 1px solid #cbd5e1;
            border-radius: 8px;
        }
        QFrame#ComplianceCard:hover {
            background-color: #f0fdf4;
            border: 1px solid #16a34a;
        }
        QHeaderView::section {"""

if target in code and "QFrame#ComplianceCard" not in code:
    code = code.replace(target, replacement, 1)
    with open("src/MainWindow.cpp", "w") as f:
        f.write(code)
    print("QSS hover style added successfully!")
