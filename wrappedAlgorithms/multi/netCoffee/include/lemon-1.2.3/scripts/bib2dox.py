#! /usr/bin/env python
"""
  BibTeX to Doxygen converter
  Usage: python bib2dox.py bibfile.bib > bibfile.dox

  This file is a part of LEMON, a generic C++ optimization library.

  **********************************************************************

  This code is the modification of the BibTeX to XML converter
  by Vidar Bronken Gundersen et al.
  See the original copyright notices below. 

  **********************************************************************

  Decoder for bibliographic data, BibTeX
  Usage: python bibtex2xml.py bibfile.bib > bibfile.xml

  v.8
  (c)2002-06-23 Vidar Bronken Gundersen
  http://bibtexml.sf.net/
  Reuse approved as long as this notification is kept.
  Licence: GPL.

  Contributions/thanks to:
  Egon Willighagen, http://sf.net/projects/jreferences/
  Richard Mahoney (for providing a test case)

  Editted by Sara Sprenkle to be more robust and handle more bibtex features.
  (c) 2003-01-15

  1.  Changed bibtex: tags to bibxml: tags.
  2.  Use xmlns:bibxml="http://bibtexml.sf.net/"
  3.  Allow spaces between @type and first {
  4.  "author" fields with multiple authors split by " and "
      are put in separate xml "bibxml:author" tags.
  5.  Option for Titles: words are capitalized
      only if first letter in title or capitalized inside braces
  6.  Removes braces from within field values
  7.  Ignores comments in bibtex file (including @comment{ or % )
  8.  Replaces some special latex tags, e.g., replaces ~ with '&#160;'
  9.  Handles bibtex @string abbreviations
        --> includes bibtex's default abbreviations for months
        --> does concatenation of abbr # " more " and " more " # abbr
  10. Handles @type( ... ) or @type{ ... }
  11. The keywords field is split on , or ; and put into separate xml
      "bibxml:keywords" tags
  12. Ignores @preamble

  Known Limitations
  1.  Does not transform Latex encoding like math mode and special
      latex symbols.
  2.  Does not parse author fields into first and last names.
      E.g., It does not do anything special to an author whose name is
      in the form LAST_NAME, FIRST_NAME
      In "author" tag, will show up as
      <bibxml:author>LAST_NAME, FIRST_NAME</bibxml:author>
  3.  Does not handle "crossref" fields other than to print
      <bibxml:crossref>...</bibxml:crossref>
  4.  Does not inform user of the input's format errors.  You just won't
      be able to transform the file later with XSL

  You will have to manually edit the XML output if you need to handle
  these (and unknown) limitations.

"""

import string, re

# set of valid name characters
valid_name_chars = '[\w\-:]'

#
# define global regular expression variables
#
author_rex = re.compile('\s+and\s+')
rembraces_rex = re.compile('[{}]')
capitalize_rex = re.compile('({[^}]*})')

# used by bibtexkeywords(data)
keywords_rex = re.compile('[,;]')

# used by concat_line(line)
concatsplit_rex = re.compile('\s*#\s*')

# split on {, }, or " in verify_out_of_braces
delimiter_rex = re.compile('([{}"])',re.I)

field_rex = re.compile('\s*(\w*)\s*=\s*(.*)')
data_rex = re.compile('\s*(\w*)\s*=\s*([^,]*),?')

url_rex = re.compile('\\\url\{([^}]*)\}')

#
# styles for html formatting
#
divstyle = 'margin-top: -4ex; margin-left: 8em;'

#
# return the string parameter without braces
#
def transformurls(str):
    return url_rex.sub(r'<a href="\1">\1</a>', str)

#
# return the string parameter without braces
#
def removebraces(str):
    return rembraces_rex.sub('', str)

#
# latex-specific replacements
# (do this after braces were removed)
#
def latexreplacements(line):
    line = string.replace(line, '~', '&nbsp;')
    line = string.replace(line, '\\\'a', '&aacute;')
    line = string.replace(line, '\\"a', '&auml;')
    line = string.replace(line, '\\\'e', '&eacute;')
    line = string.replace(line, '\\"e', '&euml;')
    line = string.replace(line, '\\\'i', '&iacute;')
    line = string.replace(line, '\\"i', '&iuml;')
    line = string.replace(line, '\\\'o', '&oacute;')
    line = string.replace(line, '\\"o', '&ouml;')
    line = string.replace(line, '\\\'u', '&uacute;')
    line = string.replace(line, '\\"u', '&uuml;')
    line = string.replace(line, '\\H o', '&otilde;')
    line = string.replace(line, '\\H u', '&uuml;')   # &utilde; does not exist
    line = string.replace(line, '\\\'A', '&Aacute;')
    line = string.replace(line, '\\"A', '&Auml;')
    line = string.replace(line, '\\\'E', '&Eacute;')
    line = string.replace(line, '\\"E', '&Euml;')
    line = string.replace(line, '\\\'I', '&Iacute;')
    line = string.replace(line, '\\"I', '&Iuml;')
    line = string.replace(line, '\\\'O', '&Oacute;')
    line = string.replace(line, '\\"O', '&Ouml;')
    line = string.replace(line, '\\\'U', '&Uacute;')
    line = string.replace(line, '\\"U', '&Uuml;')
    line = string.replace(line, '\\H O', '&Otilde;')
    line = string.replace(line, '\\H U', '&Uuml;')   # &Utilde; does not exist

    return line

#
# copy characters form a string decoding html expressions (&xyz;)
#
def copychars(str, ifrom, count):
    result = ''
    i = ifrom
    c = 0
    html_spec = False
    while (i < len(str)) and (c < count):
        if str[i] == '&':
            html_spec = True;
            if i+1 < len(str):
                result += str[i+1]
            c += 1
            i += 2
        else:
            if not html_spec:
                if ((str[i] >= 'A') and (str[i] <= 'Z')) or \
                   ((str[i] >= 'a') and (str[i] <= 'z')):
                    result += str[i]
                    c += 1
            elif str[i] == ';':
                html_spec = False;
            i += 1
    
    return result


# 
# Handle a list of authors (separated by 'and').
# It gives back an array of the follwing values:
#  - num: the number of authors,
#  - list: the list of the author names,
#  - text: the bibtex text (separated by commas and/or 'and')
#  - abbrev: abbreviation that can be used for indicate the
#    bibliography entries
#
def bibtexauthor(data):
    result = {}
    bibtex = ''
    result['list'] = author_rex.split(data)
    result['num'] = len(result['list'])
    for i, author in enumerate(result['list']):
        # general transformations
        author = latexreplacements(removebraces(author.strip()))
        # transform "Xyz, A. B." to "A. B. Xyz"
        pos = author.find(',')
        if pos != -1:
            author = author[pos+1:].strip() + ' ' + author[:pos].strip()
        result['list'][i] = author
        bibtex += author + '#'
    bibtex = bibtex[:-1]
    if result['num'] > 1:
        ix = bibtex.rfind('#')
        if result['num'] == 2:
            bibtex = bibtex[:ix] + ' and ' + bibtex[ix+1:]
        else:
            bibtex = bibtex[:ix] + ', and ' + bibtex[ix+1:]
    bibtex = bibtex.replace('#', ', ')
    result['text'] = bibtex
    
    result['abbrev'] = ''
    for author in result['list']:
        pos = author.rfind(' ') + 1
        count = 1
        if result['num'] == 1:
            count = 3
        result['abbrev'] += copychars(author, pos, count)

    return result


#
# data = title string
# @return the capitalized title (first letter is capitalized), rest are capitalized
# only if capitalized inside braces
#
def capitalizetitle(data):
    title_list = capitalize_rex.split(data)
    title = ''
    count = 0
    for phrase in title_list:
         check = string.lstrip(phrase)

         # keep phrase's capitalization the same
         if check.find('{') == 0:
              title += removebraces(phrase)
         else:
         # first word --> capitalize first letter (after spaces)
              if count == 0:
                  title += check.capitalize()
              else:
                  title += phrase.lower()
         count = count + 1

    return title


#
# @return the bibtex for the title
# @param data --> title string
# braces are removed from title
#
def bibtextitle(data, entrytype):
    if entrytype in ('book', 'inbook'):
        title = removebraces(data.strip())
    else:
        title = removebraces(capitalizetitle(data.strip()))
    bibtex = title
    return bibtex


#
# function to compare entry lists
#
def entry_cmp(x, y):
    return cmp(x[0], y[0])


#
# print the XML for the transformed "filecont_source"
#
def bibtexdecoder(filecont_source):
    filecont = []
    file = []
    
    # want @<alphanumeric chars><spaces>{<spaces><any chars>,
    pubtype_rex = re.compile('@(\w*)\s*{\s*(.*),')
    endtype_rex = re.compile('}\s*$')
    endtag_rex = re.compile('^\s*}\s*$')

    bracefield_rex = re.compile('\s*(\w*)\s*=\s*(.*)')
    bracedata_rex = re.compile('\s*(\w*)\s*=\s*{(.*)},?')

    quotefield_rex = re.compile('\s*(\w*)\s*=\s*(.*)')
    quotedata_rex = re.compile('\s*(\w*)\s*=\s*"(.*)",?')

    for line in filecont_source:
        line = line[:-1]

        # encode character entities
        line = string.replace(line, '&', '&amp;')
        line = string.replace(line, '<', '&lt;')
        line = string.replace(line, '>', '&gt;')

        # start entry: publication type (store for later use)
        if pubtype_rex.match(line):
        # want @<alphanumeric chars><spaces>{<spaces><any chars>,
            entrycont = {}
            entry = []
            entrytype = pubtype_rex.sub('\g<1>',line)
            entrytype = string.lower(entrytype)
            entryid   = pubtype_rex.sub('\g<2>', line)

        # end entry if just a }
        elif endtype_rex.match(line):
            # generate doxygen code for the entry

            # enty type related formattings
            if entrytype in ('book', 'inbook'):
                entrycont['title'] = '<em>' + entrycont['title'] + '</em>'
                if not entrycont.has_key('author'):
                    entrycont['author'] = entrycont['editor']
                    entrycont['author']['text'] += ', editors'
            elif entrytype == 'article':
                entrycont['journal'] = '<em>' + entrycont['journal'] + '</em>'
            elif entrytype in ('inproceedings', 'incollection', 'conference'):
                entrycont['booktitle'] = '<em>' + entrycont['booktitle'] + '</em>'
            elif entrytype == 'techreport':
                if not entrycont.has_key('type'):
                    entrycont['type'] = 'Technical report'
            elif entrytype == 'mastersthesis':
                entrycont['type'] = 'Master\'s thesis'
            elif entrytype == 'phdthesis':
                entrycont['type'] = 'PhD thesis'

            for eline in entrycont:
                if eline != '':
                    eline = latexreplacements(eline)

            if entrycont.has_key('pages') and (entrycont['pages'] != ''):
                entrycont['pages'] = string.replace(entrycont['pages'], '--', '-')

            if entrycont.has_key('author') and (entrycont['author'] != ''):
                entry.append(entrycont['author']['text'] + '.')
            if entrycont.has_key('title') and (entrycont['title'] != ''):
                entry.append(entrycont['title'] + '.')
            if entrycont.has_key('journal') and (entrycont['journal'] != ''):
                entry.append(entrycont['journal'] + ',')
            if entrycont.has_key('booktitle') and (entrycont['booktitle'] != ''):
                entry.append('In ' + entrycont['booktitle'] + ',')
            if entrycont.has_key('type') and (entrycont['type'] != ''):
                eline = entrycont['type']
                if entrycont.has_key('number') and (entrycont['number'] != ''):
                    eline += ' ' + entrycont['number']
                eline += ','
                entry.append(eline)
            if entrycont.has_key('institution') and (entrycont['institution'] != ''):
                entry.append(entrycont['institution'] + ',')
            if entrycont.has_key('publisher') and (entrycont['publisher'] != ''):
                entry.append(entrycont['publisher'] + ',')
            if entrycont.has_key('school') and (entrycont['school'] != ''):
                entry.append(entrycont['school'] + ',')
            if entrycont.has_key('address') and (entrycont['address'] != ''):
                entry.append(entrycont['address'] + ',')
            if entrycont.has_key('edition') and (entrycont['edition'] != ''):
                entry.append(entrycont['edition'] + ' edition,')
            if entrycont.has_key('howpublished') and (entrycont['howpublished'] != ''):
                entry.append(entrycont['howpublished'] + ',')
            if entrycont.has_key('volume') and (entrycont['volume'] != ''):
                eline = entrycont['volume'];
                if entrycont.has_key('number') and (entrycont['number'] != ''):
                    eline += '(' + entrycont['number'] + ')'
                if entrycont.has_key('pages') and (entrycont['pages'] != ''):
                    eline += ':' + entrycont['pages']
                eline += ','
                entry.append(eline)
            else:
                if entrycont.has_key('pages') and (entrycont['pages'] != ''):
                    entry.append('pages ' + entrycont['pages'] + ',')
            if entrycont.has_key('year') and (entrycont['year'] != ''):
                if entrycont.has_key('month') and (entrycont['month'] != ''):
                    entry.append(entrycont['month'] + ' ' + entrycont['year'] + '.')
                else:
                    entry.append(entrycont['year'] + '.')
            if entrycont.has_key('note') and (entrycont['note'] != ''):
                entry.append(entrycont['note'] + '.')
            if entrycont.has_key('url') and (entrycont['url'] != ''):
                entry.append(entrycont['url'] + '.')

            # generate keys for sorting and for the output
            sortkey = ''
            bibkey = ''
            if entrycont.has_key('author'):
                for author in entrycont['author']['list']:
                    sortkey += copychars(author, author.rfind(' ')+1, len(author))
                bibkey = entrycont['author']['abbrev']
            else:
                bibkey = 'x'
            if entrycont.has_key('year'):
                sortkey += entrycont['year']
                bibkey += entrycont['year'][-2:]
            if entrycont.has_key('title'):
                sortkey += entrycont['title']
            if entrycont.has_key('key'):
                sortkey = entrycont['key'] + sortkey
                bibkey = entrycont['key']
            entry.insert(0, sortkey)
            entry.insert(1, bibkey)
            entry.insert(2, entryid)
           
            # add the entry to the file contents
            filecont.append(entry)

        else:
            # field, publication info
            field = ''
            data = ''
            
            # field = {data} entries
            if bracedata_rex.match(line):
                field = bracefield_rex.sub('\g<1>', line)
                field = string.lower(field)
                data =  bracedata_rex.sub('\g<2>', line)

            # field = "data" entries
            elif quotedata_rex.match(line):
                field = quotefield_rex.sub('\g<1>', line)
                field = string.lower(field)
                data =  quotedata_rex.sub('\g<2>', line)

            # field = data entries
            elif data_rex.match(line):
                field = field_rex.sub('\g<1>', line)
                field = string.lower(field)
                data =  data_rex.sub('\g<2>', line)

            if field == 'url':
                data = '\\url{' + data.strip() + '}'
            
            if field in ('author', 'editor'):
                entrycont[field] = bibtexauthor(data)
                line = ''
            elif field == 'title':
                line = bibtextitle(data, entrytype)
            elif field != '':
                line = removebraces(transformurls(data.strip()))

            if line != '':
                line = latexreplacements(line)
                entrycont[field] = line


    # sort entries
    filecont.sort(entry_cmp)
    
    # count the bibtex keys
    keytable = {}
    counttable = {}
    for entry in filecont:
        bibkey = entry[1]
        if not keytable.has_key(bibkey):
            keytable[bibkey] = 1
        else:
            keytable[bibkey] += 1

    for bibkey in keytable.keys():
        counttable[bibkey] = 0
    
    # generate output
    for entry in filecont:
        # generate output key form the bibtex key
        bibkey = entry[1]
        entryid = entry[2]
        if keytable[bibkey] == 1:
            outkey = bibkey
        else:
            outkey = bibkey + chr(97 + counttable[bibkey])
        counttable[bibkey] += 1
        
        # append the entry code to the output
        file.append('\\section ' + entryid + ' [' + outkey + ']')
        file.append('<div style="' + divstyle + '">')
        for line in entry[3:]:
            file.append(line)
        file.append('</div>')
        file.append('')

    return file


#
# return 1 iff abbr is in line but not inside braces or quotes
# assumes that abbr appears only once on the line (out of braces and quotes)
#
def verify_out_of_braces(line, abbr):

    phrase_split = delimiter_rex.split(line)

    abbr_rex = re.compile( '\\b' + abbr + '\\b', re.I)

    open_brace = 0
    open_quote = 0

    for phrase in phrase_split:
        if phrase == "{":
            open_brace = open_brace + 1
        elif phrase == "}":
            open_brace = open_brace - 1
        elif phrase == '"':
            if open_quote == 1:
                open_quote = 0
            else:
                open_quote = 1
        elif abbr_rex.search(phrase):
            if open_brace == 0 and open_quote == 0:
                return 1

    return 0


#
# a line in the form phrase1 # phrase2 # ... # phrasen
# is returned as phrase1 phrase2 ... phrasen
# with the correct punctuation
# Bug: Doesn't always work with multiple abbreviations plugged in
#
def concat_line(line):
    # only look at part after equals
    field = field_rex.sub('\g<1>',line)
    rest = field_rex.sub('\g<2>',line)

    concat_line = field + ' ='

    pound_split = concatsplit_rex.split(rest)

    phrase_count = 0
    length = len(pound_split)

    for phrase in pound_split:
        phrase = phrase.strip()
        if phrase_count != 0:
            if phrase.startswith('"') or phrase.startswith('{'):
                phrase = phrase[1:]
        elif phrase.startswith('"'):
            phrase = phrase.replace('"','{',1)

        if phrase_count != length-1:
            if phrase.endswith('"') or phrase.endswith('}'):
                phrase = phrase[:-1]
        else:
            if phrase.endswith('"'):
                phrase = phrase[:-1]
                phrase = phrase + "}"
            elif phrase.endswith('",'):
                phrase = phrase[:-2]
                phrase = phrase + "},"

        # if phrase did have \#, add the \# back
        if phrase.endswith('\\'):
            phrase = phrase + "#"
        concat_line = concat_line + ' ' + phrase

        phrase_count = phrase_count + 1

    return concat_line


#
# substitute abbreviations into filecont
# @param filecont_source - string of data from file
#
def bibtex_replace_abbreviations(filecont_source):
    filecont = filecont_source.splitlines()

    #  These are defined in bibtex, so we'll define them too
    abbr_list = ['jan','feb','mar','apr','may','jun',
                 'jul','aug','sep','oct','nov','dec']
    value_list = ['January','February','March','April',
                  'May','June','July','August','September',
                  'October','November','December']

    abbr_rex = []
    total_abbr_count = 0

    front = '\\b'
    back = '(,?)\\b'

    for x in abbr_list:
        abbr_rex.append( re.compile( front + abbr_list[total_abbr_count] + back, re.I ) )
        total_abbr_count = total_abbr_count + 1


    abbrdef_rex = re.compile('\s*@string\s*{\s*('+ valid_name_chars +'*)\s*=(.*)',
                             re.I)

    comment_rex = re.compile('@comment\s*{',re.I)
    preamble_rex = re.compile('@preamble\s*{',re.I)

    waiting_for_end_string = 0
    i = 0
    filecont2 = ''

    for line in filecont:
        if line == ' ' or line == '':
            continue

        if waiting_for_end_string:
            if re.search('}',line):
                waiting_for_end_string = 0
                continue

        if abbrdef_rex.search(line):
            abbr = abbrdef_rex.sub('\g<1>', line)

            if abbr_list.count(abbr) == 0:
                val = abbrdef_rex.sub('\g<2>', line)
                abbr_list.append(abbr)
                value_list.append(string.strip(val))
                abbr_rex.append( re.compile( front + abbr_list[total_abbr_count] + back, re.I ) )
                total_abbr_count = total_abbr_count + 1
            waiting_for_end_string = 1
            continue

        if comment_rex.search(line):
            waiting_for_end_string = 1
            continue

        if preamble_rex.search(line):
            waiting_for_end_string = 1
            continue


        # replace subsequent abbreviations with the value
        abbr_count = 0

        for x in abbr_list:

            if abbr_rex[abbr_count].search(line):
                if verify_out_of_braces(line,abbr_list[abbr_count]) == 1:
                    line = abbr_rex[abbr_count].sub( value_list[abbr_count] + '\g<1>', line)
                # Check for # concatenations
                if concatsplit_rex.search(line):
                    line = concat_line(line)
            abbr_count = abbr_count + 1


        filecont2 = filecont2 + line + '\n'
        i = i+1


    # Do one final pass over file

    # make sure that didn't end up with {" or }" after the substitution
    filecont2 = filecont2.replace('{"','{{')
    filecont2 = filecont2.replace('"}','}}')

    afterquotevalue_rex = re.compile('"\s*,\s*')
    afterbrace_rex = re.compile('"\s*}')
    afterbracevalue_rex = re.compile('(=\s*{[^=]*)},\s*')

    # add new lines to data that changed because of abbreviation substitutions
    filecont2 = afterquotevalue_rex.sub('",\n', filecont2)
    filecont2 = afterbrace_rex.sub('"\n}', filecont2)
    filecont2 = afterbracevalue_rex.sub('\g<1>},\n', filecont2)

    return filecont2

#
# convert @type( ... ) to @type{ ... }
#
def no_outer_parens(filecont):

    # do checking for open parens
    # will convert to braces
    paren_split = re.split('([(){}])',filecont)

    open_paren_count = 0
    open_type = 0
    look_next = 0

    # rebuild filecont
    filecont = ''

    at_rex = re.compile('@\w*')

    for phrase in paren_split:
        if look_next == 1:
            if phrase == '(':
                phrase = '{'
                open_paren_count = open_paren_count + 1
            else:
                open_type = 0
            look_next = 0

        if phrase == '(':
            open_paren_count = open_paren_count + 1

        elif phrase == ')':
            open_paren_count = open_paren_count - 1
            if open_type == 1 and open_paren_count == 0:
                phrase = '}'
                open_type = 0

        elif at_rex.search( phrase ):
            open_type = 1
            look_next = 1

        filecont = filecont + phrase

    return filecont


#
# make all whitespace into just one space
# format the bibtex file into a usable form.
#
def bibtexwasher(filecont_source):

    space_rex = re.compile('\s+')
    comment_rex = re.compile('\s*%')

    filecont = []

    # remove trailing and excessive whitespace
    # ignore comments
    for line in filecont_source:
        line = string.strip(line)
        line = space_rex.sub(' ', line)
        # ignore comments
        if not comment_rex.match(line) and line != '':
            filecont.append(' '+ line)

    filecont = string.join(filecont, '')

    # the file is in one long string

    filecont = no_outer_parens(filecont)

    #
    # split lines according to preferred syntax scheme
    #
    filecont = re.sub('(=\s*{[^=]*)},', '\g<1>},\n', filecont)

    # add new lines after commas that are after values
    filecont = re.sub('"\s*,', '",\n', filecont)
    filecont = re.sub('=\s*([\w\d]+)\s*,', '= \g<1>,\n', filecont)
    filecont = re.sub('(@\w*)\s*({(\s*)[^,\s]*)\s*,',
                          '\n\n\g<1>\g<2>,\n', filecont)

    # add new lines after }
    filecont = re.sub('"\s*}','"\n}\n', filecont)
    filecont = re.sub('}\s*,','},\n', filecont)


    filecont = re.sub('@(\w*)', '\n@\g<1>', filecont)

    # character encoding, reserved latex characters
    filecont = re.sub('{\\\&}', '&', filecont)
    filecont = re.sub('\\\&', '&', filecont)

    # do checking for open braces to get format correct
    open_brace_count = 0
    brace_split = re.split('([{}])',filecont)

    # rebuild filecont
    filecont = ''

    for phrase in brace_split:
        if phrase == '{':
            open_brace_count = open_brace_count + 1
        elif phrase == '}':
            open_brace_count = open_brace_count - 1
            if open_brace_count == 0:
                filecont = filecont + '\n'

        filecont = filecont + phrase

    filecont2 = bibtex_replace_abbreviations(filecont)

    # gather
    filecont = filecont2.splitlines()
    i=0
    j=0         # count the number of blank lines
    for line in filecont:
        # ignore blank lines
        if line == '' or line == ' ':
            j = j+1
            continue
        filecont[i] = line + '\n'
        i = i+1

    # get rid of the extra stuff at the end of the array
    # (The extra stuff are duplicates that are in the array because
    # blank lines were removed.)
    length = len( filecont)
    filecont[length-j:length] = []

    return filecont


def filehandler(filepath):
    try:
        fd = open(filepath, 'r')
        filecont_source = fd.readlines()
        fd.close()
    except:
        print 'Could not open file:', filepath
    washeddata = bibtexwasher(filecont_source)
    outdata = bibtexdecoder(washeddata)
    print '/**'
    print '\page references References'
    print
    for line in outdata:
        print line
    print '*/'


# main program

def main():
    import sys
    if sys.argv[1:]:
        filepath = sys.argv[1]
    else:
        print "No input file"
        sys.exit()
    filehandler(filepath)

if __name__ == "__main__": main()


# end python script
