# Book-parser
Chunk books from folder to 4096 tokens ( 5pages ), send it to mempalace, rewrite the book in different folder using AI

this app is coded in C++ (no python code) and shell script on raspberry pi 4
1. it looks for books in .doc .docx .odt .txt .xls .xlsx .pdf.
2. it chunks the book into 4096 tokens ( 5 pages ) and stores it in .txt format in another folder.
3. shell command every .txt file and send it to mempalace app local AI memmory ( https://github.com/milla-jovovich/mempalace ). information pf previous and next header chapter, previous and next subchapter, and previous and next subsubchapter, previous and next subtitle chapter information about the book  is memorised in mempalace
4. qwen coder local llm AI parse mempalace information and writes summary
5. the workflow is operated by AI agent n8n
6. thenworkflow is operated by AI agent openclaw
