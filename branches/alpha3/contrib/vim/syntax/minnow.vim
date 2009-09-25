" Vim syntax file
" Language:	Minnow
" Maintainer:	Jason Turner <jason 'at' emptycrate com>

syn case match

" syncing method
syn sync fromstart

" Operators
syn match minnowOperator '[+-<]\|<+\|+>\|<<\|:\|::\|*\|/\|&&\|||'

" Actor declaration
syn region minnowActorBlock transparent fold matchgroup=minnowActor start="\<actor\>" end="\<end\>" contains=ALLBUT,minnowTodo

" feature declaration
syn region minnowFeatureBlock transparent fold matchgroup=minnowFeature start="\<feature\>" end="\<end\>" contains=ALLBUT,minnowTodo

" Function declaration
syn region minnowFunctionBlock transparent matchgroup=minnowFunction start="\<def\>" end="\<end\>" contains=ALLBUT,minnowTodo

" Function declaration
syn region minnowFunctionBlock transparent matchgroup=minnowFunction start="\<action\>" end="\<end\>" contains=ALLBUT,minnowTodo

" while ... do
syn region minnowRepeatBlock transparent matchgroup=minnowRepeat start="\<while\>" end="\<end\>" contains=ALL skipwhite skipempty fold

" Strings
syn region minnowString  start=+"+ end=+"+ skip=+\\\\\|\\"+ contains=minnowSpecial,@Spell



" if ... end
syn region minnowCondIfEnd contained transparent matchgroup=minnowCond start="\<if\>" end="\<end\>" contains=ALLBUT,minnowTodo

syn keyword minnowCond else elseif 


" integer number
syn match minnowNumber "\<\d\+\>"
" floating point number, with dot, optional exponent
syn match minnowFloat  "\<\d\+\.\d*\%(e[-+]\=\d\+\)\=\>"
" floating point number, starting with a dot, optional exponent
syn match minnowFloat  "\.\d\+\%(e[-+]\=\d\+\)\=\>"
" floating point number, without dot, with exponent
syn match minnowFloat  "\<\d\+e[-+]\=\d\+\>"

syn match minnowNumber "\<0x\x\+\>"

"Types
syn keyword minnowType int float double char string pointer object bool

"Keyword
syn keyword minnowKeyword isolated extern var namespace true false return

"Built in funcs
syn keyword minnowFunc print to_string is_null to_int to_float from_int from_string from_float to_double from_double

" Comments
syn match   minnowComment          "//.*$" contains=@Spell
syn region  minnowComment        matchgroup=minnowComment start="/\*" end="\*/" contains=@Spell



command -nargs=+ HiLink hi def link <args>

HiLink minnowKeyword            Keyword
HiLink minnowActor              Structure
HiLink minnowFeature            Structure
HiLink minnowStatement		Statement
HiLink minnowRepeat		Repeat
HiLink minnowString		String
HiLink minnowString2		String
HiLink minnowNumber		Number
HiLink minnowFloat		Float
HiLink minnowOperator		Operator
HiLink minnowConstant		Constant
HiLink minnowCond		Conditional
HiLink minnowFunction		Function
HiLink minnowComment		Comment
HiLink minnowTodo		Todo
HiLink minnowError		Error
HiLink minnowSpecial		SpecialChar
HiLink minnowFunc		Identifier
HiLink minnowType               Type

delcommand HiLink

let b:current_syntax = "minnow"

" vim: nowrap sw=2 sts=2 ts=8 noet ff=unix:
