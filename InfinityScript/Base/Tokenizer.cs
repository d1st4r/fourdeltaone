using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace InfinityScript
{
    public class Tokenizer
    {
        private string _text;
        private int _i;

        public Tokenizer(string text)
        {
            _text = text;
            _i = 0;
        }

        public string ReadToken()
        {
            return ReadToken(true);
        }

        public string ReadToken(bool allowNewLines)
        {
            // safety check
            if (_i >= _text.Length)
            {
                return "";
            }

            // skip whitespace and comments
            while (true)
            {
                // skip whitespace
                bool newline = false;

                while (_i < _text.Length && _text[_i] <= ' ')
                {
                    if (_i >= _text.Length)
                    {
                        return "";
                    }

                    if (_text[_i] == '\n')
                    {
                        newline = true;
                    }

                    _i++;
                }

                if (!allowNewLines && newline)
                {
                    return "";
                }

                // skip comments
                if (_i < (_text.Length - 1))
                {
                    if (_text[_i] == '/' && _text[_i + 1] == '/')
                    {
                        _i += 2;

                        while (_i < _text.Length && _text[_i] != '\n')
                        {
                            _i++;
                        }

                        continue;
                    }
                    else if (_text[_i] == '/' && _text[_i + 1] == '*')
                    {
                        _i += 2;

                        while (_i < (_text.Length - 1) && (_text[_i] != '*' || _text[_i + 1] != '/'))
                        {
                            _i++;
                        }

                        _i += 2;

                        continue;
                    }
                }

                break;
            }

            // safety check #2
            if (_i >= _text.Length)
            {
                return "";
            }

            StringBuilder token = new StringBuilder();

            if (_text[_i] == '"')
            {
                _i++;

                while (true)
                {
                    char c = _text[_i];
                    _i++;

                    if (_i >= _text.Length)
                    {
                        token.Append(c);
                    }

                    if (_i >= _text.Length || c == '"')
                    {
                        return token.ToString();
                    }

                    token.Append(c);
                }
            }
            else
            {
                while (true)
                {
                    char c = _text[_i];
                    _i++;

                    if (_i >= _text.Length)
                    {
                        token.Append(c);
                    }

                    if (_i >= _text.Length || c <= ' ')
                    {
                        return token.ToString();
                    }

                    token.Append(c);
                }
            }
        }
    }
}
