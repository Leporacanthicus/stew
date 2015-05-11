#ifndef LINEPARSER_H
#define LINEPARSER_H

class LineParser
{
public:
    LineParser(const std::string& ln);
    bool Accept(char c);
    void Expect(char c);
    char Peek();
    char Get();
    void Save();
    void Restore();
    bool Done();
    void SkipSpaces();
    void Error(const std::string& msg);
    virtual void ErrOutput(const std::string& msg) = 0;
    std::string GetWord();
    bool GetNum(uint32_t& value, int base = 0);
    virtual bool IsSeparator(const char c) = 0;
    std::string Line() { return line; }
    void SetLine(const std::string& ln) { line = ln; }
private:
    std::string line;
    std::string::size_type pos;
    std::string::size_type save_pos;
};

#endif
