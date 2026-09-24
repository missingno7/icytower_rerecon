"""Find bounded function bodies without matching braces in comments/strings."""
import hashlib
import re


def sanitized(text):
    pattern=r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"|\x27(?:\\.|[^\x27\\])*\x27'
    return re.sub(pattern,lambda m: ''.join('\n' if c=='\n' else ' ' for c in m[0]),text,flags=re.S)


def function_span(text,name):
    clean=sanitized(text)
    matches=[]
    for match in re.finditer(r'\b'+re.escape(name)+r'\s*\(',clean):
        start=match.start(); pos=match.end(); depth=1
        while pos<len(clean) and depth:
            depth += (clean[pos]=='(')-(clean[pos]==')'); pos+=1
        while pos<len(clean) and clean[pos].isspace(): pos+=1
        if pos>=len(clean) or clean[pos]!='{': continue
        # Calls inside another body are never declarations.
        if clean[:start].count('{')!=clean[:start].count('}'): continue
        depth=1; end=pos+1
        while end<len(clean) and depth:
            depth += (clean[end]=='{')-(clean[end]=='}'); end+=1
        if depth: raise ValueError('Unclosed body '+name)
        matches.append((pos,end))
    if len(matches)!=1: raise ValueError('Expected one function body for %s; found %d'%(name,len(matches)))
    return matches[0]


def body_hash(text,name):
    start,end=function_span(text,name)
    return hashlib.sha256(text[start:end].encode('cp1252')).hexdigest()
