import * as vscode from "vscode";

export function activate(context: vscode.ExtensionContext) {
  const provider = vscode.languages.registerCompletionItemProvider(
    "tmake",
    {
      provideCompletionItems(
        document: vscode.TextDocument,
        position: vscode.Position
      ): vscode.CompletionItem[] {
        const items: vscode.CompletionItem[] = [];

        // Built-in functions
        const compiler = new vscode.CompletionItem(
          "COMPILER",
          vscode.CompletionItemKind.Function
        );
        compiler.insertText = new vscode.SnippetString(
          'COMPILER("${1:g++}")'
        );
        compiler.documentation = "Set the compiler (default: g++)";
        items.push(compiler);

        const version = new vscode.CompletionItem(
          "VERSION",
          vscode.CompletionItemKind.Function
        );
        version.insertText = new vscode.SnippetString(
          'VERSION("${1:17}")'
        );
        version.documentation = "Set the C++ standard version (default: 17)";
        items.push(version);

        const program = new vscode.CompletionItem(
          "PROGRAM",
          vscode.CompletionItemKind.Function
        );
        program.insertText = new vscode.SnippetString(
          "PROGRAM(${1:name}, ${2:files}, ${3:flags}, ${4:output_dir})"
        );
        program.documentation =
          "Register a build target: PROGRAM(name, files, flags, output_dir)";
        items.push(program);

        const print = new vscode.CompletionItem(
          "PRINT",
          vscode.CompletionItemKind.Function
        );
        print.insertText = new vscode.SnippetString(
          'PRINT("${1:message}")'
        );
        print.documentation = "Print a message to the console";
        items.push(print);

        // Keywords
        const varKeyword = new vscode.CompletionItem(
          "var",
          vscode.CompletionItemKind.Keyword
        );
        varKeyword.insertText = new vscode.SnippetString(
          "var ${1:name} = ${2:value}"
        );
        varKeyword.documentation = "Declare a variable";
        items.push(varKeyword);

        const ifKeyword = new vscode.CompletionItem(
          "if",
          vscode.CompletionItemKind.Keyword
        );
        ifKeyword.insertText = new vscode.SnippetString(
          'if ${1:condition} {\n\t${2}\n}'
        );
        ifKeyword.documentation = "If statement";
        items.push(ifKeyword);

        const elseKeyword = new vscode.CompletionItem(
          "else",
          vscode.CompletionItemKind.Keyword
        );
        elseKeyword.insertText = new vscode.SnippetString(
          "else {\n\t${1}\n}"
        );
        elseKeyword.documentation = "Else block";
        items.push(elseKeyword);

        // Collect variable names from the document for autocomplete
        const text = document.getText();
        const varRegex = /\bvar\s+([a-zA-Z_]\w*)\b/g;
        const seen = new Set<string>();
        let match: RegExpExecArray | null;
        while ((match = varRegex.exec(text)) !== null) {
          const name = match[1];
          if (!seen.has(name)) {
            seen.add(name);
            const v = new vscode.CompletionItem(
              name,
              vscode.CompletionItemKind.Variable
            );
            v.documentation = `Variable: ${name}`;
            items.push(v);
          }
        }

        return items;
      },
    }
  );

  context.subscriptions.push(provider);
}

export function deactivate() {}
