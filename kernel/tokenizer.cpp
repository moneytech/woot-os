#include <string.hpp>
#include <tokenizer.hpp>

Tokenizer::Tokenizer(const char *string, const char *delims, size_t maxTokens) :
    stringCopy(String::Duplicate(string))
{
    char *nextp = nullptr;
    for(int i = 0; ; ++i)
    {
        bool tail = maxTokens && i >= (maxTokens - 1);
        const char *del = (nextp ? nextp[0] == '"' : stringCopy[0] == '"') ? "\"" : delims;
        char *s = tail ? nextp : String::Tokenize(i ? nullptr : stringCopy, del, &nextp);
        if(!s) break;

        // trim any possible leading delimiters in tail
        if(tail)
        {
            for(; *s; ++s)
            {
                bool done = true;
                for(int j = 0; delims[j]; ++j)
                {
                    if(*s == delims[j])
                    {
                        done = false;
                        break;
                    }
                }
                if(done) break;
            }
        }

        Tokens.Append(Token(s, s - stringCopy));
        if(tail) break;
    }
}

char *Tokenizer::operator[](uint idx)
{
    return Tokens[idx].String;
}

Tokenizer::~Tokenizer()
{
    delete[] stringCopy;
}
