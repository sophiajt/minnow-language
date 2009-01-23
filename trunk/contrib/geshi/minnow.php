<?php
/*************************************************************************************
 * minnow.php
 * --------
 * Author: Jonathan Turner
 * Copyright: (c) Jonathan Turner borrowed from (c) 2007 Moises Deniz
 * Release Version: 1\.0\.0
 * Date Started: 2008/08/31
 *
 * Minnow language file for GeSHi.
 *
 * CHANGES
 * -------
 * 2008/08/31 (1.0.0)
 *   -  Initial release
 *
 *************************************************************************************
 *
 *   This file is part of GeSHi.
 *
 *   GeSHi is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   GeSHi is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with GeSHi; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ************************************************************************************/

$language_data = array (
    'LANG_NAME' => 'Minnow',
    'COMMENT_SINGLE' => array(1 => "//"),
    'COMMENT_MULTI' => array("/*" => "*/"),
    'CASE_KEYWORDS' => GESHI_CAPS_NO_CHANGE,
    'QUOTEMARKS' => array('"', '`','\''),
    'ESCAPE_CHAR' => '\\',
    'KEYWORDS' => array(
        1 => array(
                'actor', 'action', 'break', 'catch',
                'def', 'done', 'else', 'elseif', 'end', 'extern', 'feature', 
                'for', 'if', 'in', 'isolated', 'new', 'spawn', 'while', 'var', 
                'namespace', 'use', 'to', 'throw', 'try'
            ),
        2 => array(
                '__FILE__', '__LINE__', 'false', 'this', 'true', 'enum',
                'return', 'int', 'uint', 'float', 'long', 'double', 'char', 'bool', 'object', 'pointer', 'string'
            ),
        3 => array('List', 'Array'
            ),
        4 => array('global', 'copy', 'delete', 'exception'
            ),
        ),
    'SYMBOLS' => array(
        '(', ')', '[', ']', '{', '}', '%', '&', '*', '|', '/', '<', '>',
        '+', '-', '<+', '+>', '<<', ':', '::'
        ),
    'CASE_SENSITIVE' => array(
        GESHI_COMMENTS => false,
        1 => false,
        2 => false,
        3 => false,
        4 => true,
        ),
    'STYLES' => array(
        'KEYWORDS' => array(
            1 => 'color:#9966CC; font-weight:bold;',
            2 => 'color:#0000FF; font-weight:bold;',
            3 => 'color:#CC0066; font-weight:bold;',
            4 => 'color:#CC00FF; font-weight:bold;',
            ),
        'COMMENTS' => array(
            1 => 'color:#008000; font-style:italic;',
            'MULTI' => 'color:#000080; font-style:italic;'
            ),
        'ESCAPE_CHAR' => array(
            0 => 'color:#000099;'
            ),
        'BRACKETS' => array(
            0 => 'color:#006600; font-weight:bold;'
            ),
        'STRINGS' => array(
            0 => 'color:#996600;'
            ),
        'NUMBERS' => array(
            0 => 'color:#006666;'
            ),
        'METHODS' => array(
            1 => 'color:#9900CC;'
            ),
        'SYMBOLS' => array(
            0 => 'color:#006600; font-weight:bold;'
            ),
        'REGEXPS' => array(
            0 => 'color:#ff6633; font-weight:bold;',
            1 => 'color:#0066ff; font-weight:bold;',
            2 => 'color:#6666ff; font-weight:bold;',
            3 => 'color:#ff3333; font-weight:bold;'
            ),
        'SCRIPT' => array(
            0 => '',
            1 => '',
            2 => '',
            )
        ),
    'URLS' => array(
        1 => '',
        2 => '',
        3 => '',
        4 => ''
        ),
    'OOLANG' => true,
    'OBJECT_SPLITTERS' => array(
        1 => '.',
	2 => '::'
        ),
    'REGEXPS' => array(
        0 => array(//Variables
            GESHI_SEARCH => "([[:space:]])(\\$[a-zA-Z_][a-zA-Z0-9_]*)",
            GESHI_REPLACE => '\\2',
            GESHI_MODIFIERS => '',
            GESHI_BEFORE => '\\1',
            GESHI_AFTER => ''
            ),
        1 => array(//Arrays
            GESHI_SEARCH => "([[:space:]])(@[a-zA-Z_][a-zA-Z0-9_]*)",
            GESHI_REPLACE => '\\2',
            GESHI_MODIFIERS => '',
            GESHI_BEFORE => '\\1',
            GESHI_AFTER => ''
            ),
        2 => "([A-Z][a-zA-Z0-9_]*::)+[A-Z][a-zA-Z0-9_]*",//Static OOP symbols
        3 => array(
            GESHI_SEARCH => "([[:space:]]|\[|\()(:[a-zA-Z_][a-zA-Z0-9_]*)",
            GESHI_REPLACE => '\\2',
            GESHI_MODIFIERS => '',
            GESHI_BEFORE => '\\1',
            GESHI_AFTER => ''
            )
        ),
    'STRICT_MODE_APPLIES' => GESHI_MAYBE,
    'SCRIPT_DELIMITERS' => array(
        0 => array(
            '<%' => '%>'
            )
        ),
    'HIGHLIGHT_STRICT_BLOCK' => array(
        0 => true,
        ),
    'TAB_WIDTH' => 2
);

?>
