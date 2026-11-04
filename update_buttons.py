with open("src/MainWindow.cpp", "r") as f:
    code = f.read()

# Let's search for the old accept/reject methods or replace them cleanly
old_accept = "void MainWindow::acceptApplication()"
if old_accept in code:
    # If partial replacement is needed or if we want to overwrite the whole block cleanly, let's do it directly
    pass

