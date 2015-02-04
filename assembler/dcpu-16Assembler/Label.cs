using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace dcpu_16Assembler
{
    class Label
    {
        private static bool caseSensitiveLabels = false;

        public String nameReal;
        public String name;
        public ushort location;
        public int line;
        public String lineStr;

        public ushort labelLocation;
        public bool found = false;

        public String plus = "";

        public Label(String name, ushort location, int line, String lineStr)
        {
            this.name = name;
            this.nameReal = name;
            if (!caseSensitiveLabels) this.name = this.name.ToLower();
            this.location = location;
            this.line = line;
            this.lineStr = lineStr;
        }
    }
}
