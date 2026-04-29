const vscode = require('vscode');
const { execFileSync } = require('child_process');
const path = require('path');
const fs = require('fs');

const SUPPORTED_LANGUAGES = ['c', 'cpp'];
const CONFIG_RELATIVE_PATH = 'utils/code_format/c_cpp/.clang-format';

function activate(context) {
    function formatCode(document, range = null) {
        const workspaceFolder = vscode.workspace.getWorkspaceFolder(document.uri);
        if (!workspaceFolder) {
            vscode.window.showErrorMessage("Formatter: Not in a workspace!");
            return [];
        }

        const configPath = path.join(workspaceFolder.uri.fsPath, CONFIG_RELATIVE_PATH);
        if (!fs.existsSync(configPath)) {
            vscode.window.showErrorMessage(`Formatter: Config not found: ${CONFIG_RELATIVE_PATH}`);
            return [];
        }

        const args = [
            `-style=file:${configPath}`,
            `--assume-filename=${document.fileName}`
        ];

        // Pass line range for paste/selection formatting
        if (range) {
            // VS Code lines are 0-based, clang-format is 1-based
            args.push(`--lines=${range.start.line + 1}:${range.end.line + 1}`);
        }

        try {
            const stdout = execFileSync('clang-format', args, {
                input: document.getText(),
                encoding: 'utf8'
            });

            const fullRange = new vscode.Range(
                document.positionAt(0),
                document.positionAt(document.getText().length)
            );
            return [vscode.TextEdit.replace(fullRange, stdout)];
        } catch (error) {
            vscode.window.showErrorMessage("Clang-Format Failed: " + error.message);
            return [];
        }
    }

    // Full-document formatting (Shift+Alt+F)
    const docProvider = vscode.languages.registerDocumentFormattingEditProvider(SUPPORTED_LANGUAGES, {
        provideDocumentFormattingEdits(document) {
            return formatCode(document);
        }
    });

    // Range formatting (selection & format-on-paste)
    const rangeProvider = vscode.languages.registerDocumentRangeFormattingEditProvider(SUPPORTED_LANGUAGES, {
        provideDocumentRangeFormattingEdits(document, range) {
            return formatCode(document, range);
        }
    });

    // Format on save
    const saveListener = vscode.workspace.onWillSaveTextDocument(event => {
        const doc = event.document;
        if (!SUPPORTED_LANGUAGES.includes(doc.languageId)) return;
        event.waitUntil(Promise.resolve(formatCode(doc)));
    });

    context.subscriptions.push(docProvider, rangeProvider, saveListener);
}

function deactivate() {}

module.exports = { activate, deactivate };