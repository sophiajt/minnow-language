Install indent and syntax subdirectories to:

~/.vim/

Then either manually set syntax:

:set syntax=minnow

Or add the line:

au BufRead,BufNewFile *.mno     set filetype=minnow

To the file:

~/.vim/ftdetect

See the vim documentation on this:

http://vimdoc.sourceforge.net/htmldoc/syntax.html#mysyntaxfile
