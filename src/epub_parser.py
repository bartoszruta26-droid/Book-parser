#!/usr/bin/env python3
"""
EPUB Parser for Book-Parser System
Extracts text content from EPUB files and converts them to plain text format.
"""

import os
import sys
import json
import argparse
import html
from pathlib import Path
from ebooklib import epub
from lxml import etree, html as lxml_html


class EPUBParser:
    """Parser for EPUB files with intelligent chapter detection."""
    
    def __init__(self, verbose=False):
        self.verbose = verbose
        self.metadata = {}
        self.chapters = []
        
    def parse(self, epub_path):
        """
        Parse an EPUB file and extract its content.
        
        Args:
            epub_path: Path to the EPUB file
            
        Returns:
            dict: Dictionary containing metadata and chapters
        """
        epub_path = Path(epub_path)
        
        if not epub_path.exists():
            raise FileNotFoundError(f"EPUB file not found: {epub_path}")
            
        if self.verbose:
            print(f"Parsing EPUB: {epub_path}")
        
        # Read the EPUB file
        book = epub.read_epub(epub_path)
        
        # Extract metadata
        self._extract_metadata(book)
        
        # Extract chapters/content
        self._extract_content(book)
        
        return {
            'metadata': self.metadata,
            'chapters': self.chapters,
            'source_file': str(epub_path),
            'total_chapters': len(self.chapters)
        }
    
    def _extract_metadata(self, book):
        """Extract metadata from the EPUB book."""
        self.metadata = {
            'title': self._get_metadata(book, 'title'),
            'creator': self._get_metadata(book, 'creator'),
            'language': self._get_metadata(book, 'language'),
            'publisher': self._get_metadata(book, 'publisher'),
            'date': self._get_metadata(book, 'date'),
            'identifier': self._get_metadata(book, 'identifier'),
            'description': self._get_metadata(book, 'description'),
        }
        
        if self.verbose:
            print(f"Title: {self.metadata['title']}")
            print(f"Creator: {self.metadata['creator']}")
    
    def _get_metadata(self, book, key):
        """Get a specific metadata field from the book."""
        try:
            value = book.get_metadata('DC', key)
            if value:
                return value[0][0] if isinstance(value[0], tuple) else value[0]
        except:
            pass
        return None
    
    def _extract_content(self, book):
        """Extract text content from all documents in the EPUB."""
        self.chapters = []
        
        # Get all items of type document
        documents = [item for item in book.get_items() if item.get_type() == 9]
        
        if self.verbose:
            print(f"Found {len(documents)} documents")
        
        for idx, doc in enumerate(documents):
            chapter_content = self._process_document(doc, idx)
            if chapter_content:
                self.chapters.append(chapter_content)
    
    def _process_document(self, doc, index):
        """Process a single document and extract text."""
        try:
            # Get the content
            content = doc.get_content()
            
            # Parse HTML
            html_content = content.decode('utf-8', errors='ignore')
            
            # Remove HTML tags and extract text
            text = self._html_to_text(html_content)
            
            if not text.strip():
                return None
            
            # Try to detect chapter title
            title = self._detect_title(html_content, index)
            
            return {
                'index': index,
                'title': title,
                'content': text,
                'word_count': len(text.split()),
                'char_count': len(text)
            }
            
        except Exception as e:
            if self.verbose:
                print(f"Error processing document {index}: {e}")
            return None
    
    def _html_to_text(self, html_content):
        """Convert HTML content to plain text."""
        try:
            # Parse HTML
            doc = lxml_html.fromstring(html_content)
            
            # Remove script and style elements
            for element in doc.xpath('//script|//style|//head'):
                element.drop_tree()
            
            # Get text content
            text = doc.text_content()
            
            # Decode HTML entities
            text = html.unescape(text)
            
            # Clean up whitespace
            lines = []
            for line in text.split('\n'):
                line = line.strip()
                if line:
                    lines.append(line)
            
            return '\n\n'.join(lines)
            
        except Exception as e:
            if self.verbose:
                print(f"HTML parsing error: {e}")
            # Fallback: simple tag removal
            import re
            text = re.sub(r'<[^>]+>', ' ', html_content)
            text = html.unescape(text)
            return '\n'.join([l.strip() for l in text.split('\n') if l.strip()])
    
    def _detect_title(self, html_content, index):
        """Try to detect the chapter title from HTML."""
        try:
            doc = lxml_html.fromstring(html_content)
            
            # Try common title tags
            for tag in ['h1', 'h2', 'h3', 'title']:
                elements = doc.xpath(f'//{tag}')
                if elements:
                    title = elements[0].text_content().strip()
                    if title and len(title) < 200:
                        return title
            
            # If no title found, use generic name
            return f"Chapter {index + 1}"
            
        except:
            return f"Chapter {index + 1}"
    
    def to_plain_text(self, output_path=None):
        """
        Convert parsed content to plain text format.
        
        Args:
            output_path: Optional path to save the text file
            
        Returns:
            str: The complete text content
        """
        lines = []
        
        # Add metadata as header
        if self.metadata.get('title'):
            lines.append(f"# {self.metadata['title']}")
            lines.append("")
        if self.metadata.get('creator'):
            lines.append(f"Autor: {self.metadata['creator']}")
            lines.append("")
        
        # Add chapters
        for chapter in self.chapters:
            if chapter.get('title'):
                lines.append(f"\n## {chapter['title']}")
                lines.append("")
            lines.append(chapter.get('content', ''))
        
        full_text = '\n'.join(lines)
        
        if output_path:
            output_path = Path(output_path)
            output_path.parent.mkdir(parents=True, exist_ok=True)
            with open(output_path, 'w', encoding='utf-8') as f:
                f.write(full_text)
            
            if self.verbose:
                print(f"Saved plain text to: {output_path}")
        
        return full_text
    
    def to_json(self, output_path=None):
        """
        Convert parsed content to JSON format.
        
        Args:
            output_path: Optional path to save the JSON file
            
        Returns:
            dict: The complete data structure
        """
        data = {
            'metadata': self.metadata,
            'chapters': self.chapters,
            'source_file': self.metadata.get('source_file', ''),
            'total_chapters': len(self.chapters)
        }
        
        if output_path:
            output_path = Path(output_path)
            output_path.parent.mkdir(parents=True, exist_ok=True)
            with open(output_path, 'w', encoding='utf-8') as f:
                json.dump(data, f, indent=2, ensure_ascii=False)
            
            if self.verbose:
                print(f"Saved JSON to: {output_path}")
        
        return data


def main():
    parser = argparse.ArgumentParser(
        description='Parse EPUB files and extract text content'
    )
    parser.add_argument('input', help='Input EPUB file or directory')
    parser.add_argument('-o', '--output', help='Output directory for converted files')
    parser.add_argument('-f', '--format', choices=['txt', 'json', 'both'], 
                       default='txt', help='Output format (default: txt)')
    parser.add_argument('-v', '--verbose', action='store_true', 
                       help='Verbose output')
    
    args = parser.parse_args()
    
    input_path = Path(args.input)
    output_dir = Path(args.output) if args.output else None
    
    # Find all EPUB files
    if input_path.is_file():
        epub_files = [input_path]
    elif input_path.is_dir():
        epub_files = list(input_path.glob('**/*.epub'))
        if not epub_files:
            print(f"No EPUB files found in {input_path}")
            sys.exit(1)
    else:
        print(f"Input path does not exist: {input_path}")
        sys.exit(1)
    
    print(f"Found {len(epub_files)} EPUB file(s)")
    
    # Process each EPUB file
    for epub_file in epub_files:
        print(f"\nProcessing: {epub_file.name}")
        
        try:
            parser_obj = EPUBParser(verbose=args.verbose)
            result = parser_obj.parse(epub_file)
            
            # Determine output filename
            base_name = epub_file.stem
            if output_dir:
                output_dir.mkdir(parents=True, exist_ok=True)
                txt_output = output_dir / f"{base_name}.txt"
                json_output = output_dir / f"{base_name}.json"
            else:
                txt_output = epub_file.parent / f"{base_name}.txt"
                json_output = epub_file.parent / f"{base_name}.json"
            
            # Save in requested format
            if args.format in ['txt', 'both']:
                parser_obj.to_plain_text(txt_output)
                print(f"  ✓ Created: {txt_output}")
            
            if args.format in ['json', 'both']:
                parser_obj.to_json(json_output)
                print(f"  ✓ Created: {json_output}")
            
            # Print summary
            print(f"  Chapters: {result['total_chapters']}")
            if result['metadata'].get('title'):
                print(f"  Title: {result['metadata']['title']}")
            if result['metadata'].get('creator'):
                print(f"  Author: {result['metadata']['creator']}")
                
        except Exception as e:
            print(f"  ✗ Error: {e}")
            if args.verbose:
                import traceback
                traceback.print_exc()
    
    print("\nDone!")


if __name__ == '__main__':
    main()
