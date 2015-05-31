#include <string>
#include <cassert>
#include <climits>
#include "lineparser.h"

LineParser::LineParser(const std::string& ln)
    : line(ln), pos(0)
{
}

bool LineParser::GetNum(uint32_t& value, int base)
{
    bool negative = false;
    std::string w = GetWord();
    if (w[0] == '-')
    {
	negative = true;
	w = w.substr(1);
    }
    uint64_t tmp;
    try
    {
	tmp = std::stoul(w, 0, base);
    }
    catch(...)
    {
	return false;
    }
    if (tmp > UINT_MAX)
    {
	return false;
    }
    if (negative)
    {
	if(tmp > (uint64_t)INT_MAX+1)
	{
	    return false;
	}
	tmp = -tmp;
    }
    value = tmp;
    return true;
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
