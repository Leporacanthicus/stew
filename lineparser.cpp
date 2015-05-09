#include <string>
#include <cassert>
#include "lineparser.h"

LineParser::LineParser(const std::string& ln)
    : line(ln), pos(0)
{
}

std::string LineParser::GetWord()
{
    std::string w;
    while(!IsSeparator(line[pos]))
    {
	w += line[pos];
	pos++;
    }
    SkipSpaces();
    return w;
}

char LineParser::Get()
{
    if (pos < line.length())
    {
	char res = line[pos];
	pos++;
	return res;
    }
    return EOF;
}

char LineParser::Peek()
{
    if (pos < line.length())
    {
	return line[pos];
    }
    return EOF;
}    

bool LineParser::Done()
{
    return pos >= line.length();
}

bool LineParser::Accept(char c)
{
    assert(pos <= line.length());
    if (line[pos] == c)
    {
	pos++;
	return true;
    }
    return false;
}

void LineParser::Expect(char c)
{
    assert(pos <= line.length());
    if (line[pos] == c)
    {
	pos++;
    }
    else
    {
	Error(std::string("Unexpected character, expected ") + c);
    }
}

void LineParser::SkipSpaces()
{
    while(isspace(Peek()))
    {
	Get();
    }
}

void LineParser::Save()
{
    save_pos = pos;
}

void LineParser::Restore()
{
    pos = save_pos;
}

void LineParser::Error(const std::string& msg)
{
    ErrOutput(msg);
    pos = line.length();
}
