using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace dcpu_16Assembler
{
    class Code
    {
        public int line;
        public ushort location;
        public ushort word;

        public Code(int line, ushort location, ushort word)
        {
            this.line = line;
            this.location = location;
            this.word = word;
        }
    }
}
