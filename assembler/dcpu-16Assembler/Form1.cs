using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Collections;
using System.IO;
using System.Runtime.InteropServices;
using NCalc;





namespace dcpu_16Assembler
{
    public partial class Form1 : Form
    {
        private char[] whiteSpaceChars = { ' ', '\n', '\r', '\t' };
        private char[] commaChars = { ',' };
        private char[] commentChars = { ';' };
        private char[] labelChars = { ':' };
        private char[] bracketOpenChars = { '[', '(' };
        private char[] bracketCloseChars = { ']', ')' };
        private char[] hexChars = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

        private List<Label> labels = new List<Label>();
        private List<Label> labelsTo = new List<Label>();

        private List<String> lines = new List<String>();
        private int linesLine = 0;

        private ushort location = 0;
        private ushort locationPlus = 0;
        private int line = 0;
        private String lineStr = "";

        private bool Error = false;

        private String[] registers = { "A", "B", "C", "X", "Y", "Z", "I", "J" };


        private ushort[] memory = new ushort[0x10000];

        private List<Code> codeList = new List<Code>();


        private System.Drawing.Font richTextBoxFont = new System.Drawing.Font("COURIER", 9, System.Drawing.FontStyle.Regular);

        public Form1()
        {
            InitializeComponent();
        }

        delegate void progressBarAssembleUpdateDelegate(int value, int max);
        private void progressBarAssembleUpdate(int value, int max)
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
            if (this.progressBarAssemble.InvokeRequired)
            {
                progressBarAssembleUpdateDelegate d = new progressBarAssembleUpdateDelegate(progressBarAssembleUpdate);
                this.Invoke(d, new object[] { value, max });
            }
            else
            {
                if (max >= 0) this.progressBarAssemble.Maximum = max;
                this.progressBarAssemble.Value = value;
            }
        }

        delegate void setMemoryDelegate(String str);
        private void setMemoryText(String str)
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
            if (this.progressBarAssemble.InvokeRequired)
            {
                setMemoryDelegate d = new setMemoryDelegate(setMemoryText);
                this.Invoke(d, new object[] { str });
            }
            else
            {
                richTextBoxMemory.IsReadOnly = false;
                richTextBoxMemory.Text = str;
                richTextBoxMemory.IsReadOnly = true;

                richTextBoxMemory.Selection.Start = richTextBoxMemory.Text.Length;
                richTextBoxMemory.Scrolling.ScrollToCaret();
            }
        }

        delegate List<String> getEditorLinesDelegate();
        private List<String> getEditorLines()
        {
            // InvokeRequired required compares the thread ID of the
            // calling thread to the thread ID of the creating thread.
            // If these threads are different, it returns true.
            if (this.progressBarAssemble.InvokeRequired)
            {
                getEditorLinesDelegate d = new getEditorLinesDelegate(getEditorLines);
                return (List<String>)this.Invoke(d, new object[] { });
            }
            else
            {
                lines.Clear();
                linesLine = 0;

                for (int i = 0; i < richTextBoxEditor.Lines.Count; i++)
                {
                    ScintillaNET.Line l = richTextBoxEditor.Lines[i];

                    String str = l.Text;
                    lineStr = str;

                    //str = removeCommentsEtc(str);


                    lines.Add(str);
                    linesLine++;
                }

                return lines;
            }
        }

        private void assemble()
        {
            location = 0;
            line = 0;
            lineStr = "";

            labels.Clear();
            labelsTo.Clear();
            codeList.Clear();

            setMemoryText("");

            Error = false;

            memoryClear();


            lines.Clear();
            linesLine = 0;


            lines = getEditorLines();


            progressBarAssembleUpdate(0, lines.Count);
            


            for (int i = 0; i < lines.Count; i++)
            {
                String str = lines[i];

                str = removeCommentsEtc(str);

                if (str.Length > 0)
                {

                    lineStr = str;


                    //MessageBox.Show(str);

                    String label = getLabel(str, out str);

                    if (label != "")
                    {
                        //MessageBox.Show(label);
                        Label lab = new Label(label, location, line, lineStr);
                        labels.Add(lab);
                    }

                    if (str.Length > 0)
                    {

                        String op = getOp(str, out str);
                        List<String> args = getArgs(str);

                        if (op != "")
                        {
                            if (!preprocessParse(op, args))
                            {
                                parse(op, args);
                            }
                        }

                    }


                    if (Error)
                    {
                        break;
                    }
                }

                line++;

                progressBarAssembleUpdate(line, -1);

            }

            if (!Error)
            {
                computeLabels();

                writeMemory();
            }
            else
            {
                progressBarAssembleUpdate(0, -1);
            }

            //memoryDump(0, 1000);
            codeDump(lines.Count);
        }

        private void buttonCompile_Click(object sender, EventArgs e)
        {
            backgroundWorkerAssemble.RunWorkerAsync();
        }

        private String removeCommentsEtc(String str)
        {
            str = str.Trim();

            int comma = str.IndexOfAny(commentChars);
            if (comma >= 0)
            {
                str = str.Remove(comma, str.Length - comma);
            }

            return str.Trim();
        }

        private String getLabel(String sIn, out String str)
        {
            str = sIn.Trim();

            String label = "";

            if (labelChars.Contains(str[0]))
            {
                str = str.Remove(0, 1);

                int index = str.IndexOfAny(whiteSpaceChars);
                if (index >= 0)
                {
                    label = str.Remove(index, str.Length - index);

                    str = str.Remove(0, index);
                    str = str.Trim();
                }
                else
                {
                    label = str;
                    str = "";
                }
            }

            return label.Trim();
        }

        private String getOp(String sIn, out String str)
        {
            str = sIn.Trim();

            String op = str;

            int index = str.IndexOfAny(whiteSpaceChars);
            if (index >= 0)
            {
                op = str.Remove(index, str.Length - index);

                str = str.Remove(0, index);
                str = str.Trim();
            }

            return op.Trim();
        }

        private List<String> getArgs(String str)
        {
            str = str.Trim();

            List<String> args = new List<String>();

            String arg = "";
            bool treatNextAsNormal = false;
            bool inQuote = false;
            bool quote = false;

            for (int i = 0; i < str.Length; i++)
            {
                char ch = str[i];

                if (treatNextAsNormal)
                {
                    if (!(quote && !inQuote)) arg += ch;
                    treatNextAsNormal = false;
                }
                else
                {

                    switch (ch)
                    {
                        case ',':
                        {
                            if (!inQuote)
                            {
                                if (quote)
                                {
                                    args.Add(arg.Trim());
                                    quote = false;
                                }
                                else
                                {
                                    args.Add(arg.Trim());
                                }

                                arg = "";
                            }
                            else
                            {
                                if (!(quote && !inQuote)) arg += ch;
                            }
                            break;
                        }

                        case '"':
                        {
                            if (inQuote)
                            {
                                inQuote = false;
                                arg += ch;
                            }
                            else
                            {
                                inQuote = true;
                                quote = true;
                                arg += ch;
                            }
                            break;
                        }

                        case '\\':
                        {
                            if (inQuote)
                            {
                                treatNextAsNormal = true;
                            }
                            break;
                        }

                        default:
                            arg += ch;
                            break;
                    }

                }
            }

            if (arg.Trim() != "")
            {
                if (quote)
                {
                    args.Add(arg.Trim());
                    quote = false;
                }
                else
                {
                    args.Add(arg.Trim());
                }
            }

            return args;
        }

        private List<String> stringToData(String str)
        {
            List<String> data = new List<String>();

            byte[] asciiBytes = Encoding.ASCII.GetBytes(str);

            for (int i = 0; i < asciiBytes.Length; i++)
            {
                data.Add(asciiBytes[i].ToString());
            }

            return data;
        }

        private bool preprocessParse(String op, List<String> args)
        {
            bool processed = false;

            switch (op.ToUpper())
            {
                case "ORG": //.ORG n - Set the origin of the assemnly. n is an address. Eg. .ORG 0x10 means that the succeeding instructions now start at memory address 0x10.
                case ".ORG":
                    {
                        processed = true;

                        if (args.Count != 1)
                        {
                            error("Not right amount (1) of arguments for opcode.");
                        }
                        else
                        {
                            int n;
                            if (!parseNum(args[0], out n))
                            {
                                error("Couldn't understand or compute the number: " + args[0]);
                            }
                            else
                            {
                                location = (ushort)n;
                            }
                        }

                        break;
                    }

                case "DEFINE": //.DEFINE x y - If a token is x, it's replaced by y. Eg. .DEFINE MY_REGISTER A ; SET MY_REGISTER, 3 will set A to 3.
                case ".DEFINE":
                    {
                        processed = true;


                        if (args.Count != 1)
                        {
                            error("Not right amount (1) of arguments for opcode. (Word to be replaced and word to be replaced with need to be seperated by a space.)");
                        }
                        else
                        {
                            int spacePos = args[0].IndexOfAny(whiteSpaceChars);
                            String a = args[0].Remove(spacePos, args[0].Length - spacePos);
                            String b = args[0].Remove(0, spacePos);

                            for (int i = 0; i < lines.Count; i++)
                            {
                                lines[i].Replace(a, b);
                            }
                        }

                        break;
                    }

                case "INCLUDE":
                case ".INCLUDE":
                    {
                        processed = true;


                        if (args.Count != 1)
                        {
                            error("Not right amount (1) of arguments for opcode.");
                        }
                        else
                        {
                            String fname = args[0].Trim();
                            if (fname[0] == '"')
                            {
                                fname = fname.Remove(0, 1);
                                fname = fname.Remove(fname.Length - 1, 1);
                            }

                            if (!File.Exists(fname))
                            {
                                error("Couldn't find file at path: " + fname);
                            }
                            else
                            {

                                String text;
                                List<String> textList = new List<String>();

                                StreamReader streamReader = new StreamReader(fname);

                                while ((text = streamReader.ReadLine()) != null)
                                {
                                    textList.Add(text);
                                }

                                streamReader.Close();

                                lines.InsertRange(linesLine, textList);

                            }
                        }


                        break;
                    }
            }

            return processed;
        }

        private void parse(String Op, List<String> args)
        {
            locationPlus = 0;

            //if (B != "") MessageBox.Show(Op + " " + A + ", " + B);
            //else MessageBox.Show(Op + " " + A);

            List<BitArray> nextWords;

            BitArray word = parseOp(Op, args, out nextWords);


            if (word != null)
            {

                int[] array = new int[1];
                word.CopyTo(array, 0);
                ushort wordShort = (ushort)array[0];

                //MessageBox.Show("0x" + wordShort.ToString("x4"));

                Code code = new Code(line, location, wordShort);
                codeList.Add(code);

                memory[location++] = wordShort;

                for (int i = 0; i < nextWords.Count; i++)
                {
                    nextWords[i].CopyTo(array, 0);
                    wordShort = (ushort)array[0];

                    Code c = new Code(line, location, wordShort);
                    codeList.Add(c);

                    memory[location++] = wordShort;
                }

            }

        }

        private BitArray parseOp(String Op, List<String> args, out List<BitArray> nextWords)
        {
            ushort op = 0x0;
            ushort op2 = 0x0;
            BitArray word = null ;
            nextWords = new List<BitArray>();


            switch (Op.ToUpper())
            {
                case "SET":
                    op = 0x01;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "ADD":
                    op = 0x02;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "SUB":
                    op = 0x03;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "MUL":
                    op = 0x04;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "MLI":
                    op = 0x05;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "DIV":
                    op = 0x06;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "DVI":
                    op = 0x07;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "MOD":
                    op = 0x08;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "MDI":
                    op = 0x09;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "AND":
                    op = 0x0a;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "BOR":
                    op = 0x0b;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "XOR":
                    op = 0x0c;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "SHR":
                    op = 0x0d;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "ASR":
                    op = 0x0e;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "SHL":
                    op = 0x0f;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "IFB":
                    op = 0x10;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "IFC":
                    op = 0x11;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "IFE":
                    op = 0x12;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "IFN":
                    op = 0x13;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "IFG":
                    op = 0x14;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "IFA":
                    op = 0x15;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "IFL":
                    op = 0x16;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "IFU":
                    op = 0x17;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "ADX":
                    op = 0x1a;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "SBX":
                    op = 0x1b;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "STI":
                    op = 0x1e;
                    word = parseOpBasic(op, args, out nextWords);
                    break;

                case "STD":
                    op = 0x1f;
                    word = parseOpBasic(op, args, out nextWords);
                    break;


                case "JSR":
                    op = 0x0;
                    op2 = 0x01;
                    word = parseOpNonBasic(op, op2, args, out nextWords);
                    break;

                case "INT":
                    op = 0x0;
                    op2 = 0x08;
                    word = parseOpNonBasic(op, op2, args, out nextWords);
                    break;

                case "IAG":
                    op = 0x0;
                    op2 = 0x09;
                    word = parseOpNonBasic(op, op2, args, out nextWords);
                    break;

                case "IAS":
                    op = 0x0;
                    op2 = 0x0a;
                    word = parseOpNonBasic(op, op2, args, out nextWords);
                    break;

                case "RFI":
                    op = 0x0;
                    op2 = 0x0b;
                    word = parseOpNonBasic(op, op2, args, out nextWords);
                    break;

                case "IAQ":
                    op = 0x0;
                    op2 = 0x0c;
                    word = parseOpNonBasic(op, op2, args, out nextWords);
                    break;

                case "HWN":
                    op = 0x0;
                    op2 = 0x10;
                    word = parseOpNonBasic(op, op2, args, out nextWords);
                    break;

                case "HWQ":
                    op = 0x0;
                    op2 = 0x11;
                    word = parseOpNonBasic(op, op2, args, out nextWords);
                    break;

                case "HWI":
                    op = 0x0;
                    op2 = 0x12;
                    word = parseOpNonBasic(op, op2, args, out nextWords);
                    break;

                case "DAT":
                case ".DAT": //.DAT/.DW x, (,x, x, x) - Put specified words on the memory position. Literals and "strings" are allowed.
                case "DATA": case ".DATA":
                case "DW": case ".DW":
                    {
                        //convert quotes to data
                        List<String> newargs = new List<String>();
                        foreach (String s in args)
                        {
                            if (s[0] == '"')
                            {
                                String s2 = s;
                                s2 = s2.Remove(0, 1);
                                s2 = s2.Remove(s2.Length - 1, 1);

                                newargs.AddRange(stringToData(s2));
                            }
                            else
                            {
                                newargs.Add(s);
                            }
                        }

                        args = newargs;

                        for (int i = 0; i < args.Count; i++)
                        {
                            int num;
                            if (parseNum(args[i], out num))
                            {
                                int[] ints = new int[] { num };
                                BitArray w = new BitArray(ints);
                                w.Length = 16;

                                if (i == 0) word = w;
                                else nextWords.Add(w);
                            }
                            else
                            {
                                String label = args[i];

                                BitArray w = new BitArray(16);
                                if (i == 0) word = w;
                                else nextWords.Add(w);

                                Label lab = new Label(label, (ushort)(location + i), line, lineStr);
                                labelsTo.Add(lab);
                            }
                        }
                        break;
                    }

                case "RESERVE": //.RESERVE n - Reserves a memory area for eg. a text buffer. n is the size of the area.
                case ".RESERVE":
                    {
                        if (args.Count != 1)
                        {
                            error("Not right amount (1) of arguments for opcode.");
                        }
                        else
                        {
                            int n;
                            if (!parseNum(args[0], out n))
                            {
                                error("Couldn't understand the number: " + args[0]);
                            }
                            else
                            {
                                word = new BitArray(16);
                                for (int i = 1; i < n; i++)
                                {
                                    nextWords.Add(new BitArray(16));
                                }
                            }
                        }
                        break;
                    }

                case "FILL": //.FILL n x - Same as .RESERVE but fills the area with the word x. (repeating n times)
                case ".FILL":
                    {
                        if (args.Count < 2)
                        {
                            error("Not right amount (>1) of arguments for opcode.");
                        }
                        else
                        {
                            int n;
                            if (!parseNum(args[0], out n))
                            {
                                error("Couldn't understand the number: " + args[0]);
                            }
                            else
                            {
                                for (int j = 0; j < n; j++)
                                {
                                    for (int i = 1; i < args.Count; i++)
                                    {
                                        int num;
                                        if (parseNum(args[i], out num))
                                        {
                                            int[] ints = new int[] { num };
                                            BitArray w = new BitArray(ints);
                                            w.Length = 16;

                                            if (j == 0 && i == 0) word = w;
                                            else nextWords.Add(w);
                                        }
                                        else
                                        {
                                            error("No idea how to handle the fill data argument: " + args[i]);
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        break;
                    }

                case "INCBIN": //.INCBIN "file" LE|BE - Includes a binary file (path is relative to the source file) into the assembly at the current address. LE is Little Endian and BE is Big Endian.
                case ".INCBIN":
                    {
                        if (args.Count != 1)
                        {
                            error("Not right amount (1) of arguments for opcode. (\"file\" and LE|BE need to be space seperated.)");
                        }
                        else
                        {
                            int spacePos = args[0].IndexOfAny(whiteSpaceChars);
                            String fname = args[0].Remove(spacePos, args[0].Length - spacePos).Trim();
                            String LEorBE = args[0].Remove(0, spacePos).Trim().ToUpper();

                            if (fname[0] == '"')
                            {
                                fname = fname.Remove(0, 1);
                                fname = fname.Remove(fname.Length - 1, 1);
                            }

                            if (!File.Exists(fname))
                            {
                                error("Couldn't find file at path: " + fname);
                            }
                            else
                            {

                                if (LEorBE == "") LEorBE = "LE";
                                if (!(LEorBE == "LE" || LEorBE == "BE"))
                                {
                                    error("Couldn't understand whether to use Little Endian (LE) or Big Endian (BE): " + LEorBE);
                                }
                                else
                                {
                                    FileStream fs = File.OpenRead(fname);

                                    byte[] bytes = new byte[fs.Length];
                                    fs.Read(bytes, 0, Convert.ToInt32(fs.Length));
                                    fs.Close();

                                    if (bytes.Length % 2 != 0)
                                    {
                                        error("File is not made up of 16 bit words. (File length was odd.)");
                                    }
                                    else
                                    {

                                        for (int i = 0; i < bytes.Length; i += 2)
                                        {
                                            byte[] b = { bytes[i], bytes[i + 1] };

                                            ushort data;

                                            if (LEorBE == "LE")
                                            {
                                                Array.Reverse(bytes);
                                            }

                                            data = BitConverter.ToUInt16(bytes, 0);

                                            if (i == 0)
                                            {
                                                word = new BitArray(data);
                                                word.Length = 16;
                                            }
                                            else
                                            {
                                                BitArray nword = new BitArray(data);
                                                nword.Length = 16;
                                                nextWords.Add(nword);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        break;
                    }             

                default:
                    op = 0x0;
                    error("Unknown opcode: " + Op);
                    break;

                

            }

            return word;
        }

        private BitArray parseOpBasic(ushort Op, List<String> args, out List<BitArray> nextWords)
        {
            BitArray word = null;
            nextWords = new List<BitArray>();

            if (args.Count != 2)
            {
                error("Not right amount (2) arguments for opcode.");
            }
            else
            {

                BitArray nextWord = null;
                BitArray a = parseValue(args[0], out nextWord);
                if (nextWord != null)
                {
                    nextWords.Add(nextWord);
                }

                nextWord = null;
                BitArray b = parseValue(args[1], out nextWord);
                if (nextWord != null)
                {
                    nextWords.Add(nextWord);
                }


                int[] ints = { Op };
                BitArray op = new BitArray(ints);
                op.Length = 5;

                word = makeWord(op, a, b);

            }

            return word;
        }

        private BitArray parseOpNonBasic(ushort Op, ushort Op2, List<String> args, out List<BitArray> nextWords)
        {
            BitArray word = null;
            nextWords = new List<BitArray>();

            if (args.Count != 1)
            {
                error("Not right amount (1) of arguments for opcode.");
            }
            else
            {
                int[] ints = { Op2 };
                BitArray a = new BitArray(ints);
                a.Length = 5;

                BitArray nextWord = null;
                BitArray b = parseValue(args[0], out nextWord);
                if (nextWord != null)
                {
                    nextWords.Add(nextWord);
                }

                ints = new int[] { Op };
                BitArray op = new BitArray(ints);
                op.Length = 5;

                word = makeWord(op, a, b);

            }

            return word;
        }

        private BitArray makeWord(BitArray op, BitArray a, BitArray b)
        {
            BitArray word = new BitArray(16);

            for (int i = 0; i < 5; i++)
            {
                word[i] = op[i];
            }

            for (int i = 0; i < 5; i++)
            {
                word[i + 5] = a[i];
            }

            for (int i = 0; i < 6; i++)
            {
                word[i + 10] = b[i];
            }

            return word;
        }

        private BitArray parseValue(String V, out BitArray nextWord)
        {
            ushort v = 0x0;
            nextWord = null;

            //MessageBox.Show("V: " + V);

            switch (V.ToUpper())
            {
                case "A":
                    v = 0x00;
                    break;

                case "B":
                    v = 0x01;
                    break;

                case "C":
                    v = 0x02;
                    break;

                case "X":
                    v = 0x03;
                    break;

                case "Y":
                    v = 0x04;
                    break;

                case "Z":
                    v = 0x05;
                    break;

                case "I":
                    v = 0x06;
                    break;

                case "J":
                    v = 0x07;
                    break;

                case "[A]":
                    v = 0x08;
                    break;

                case "[B]":
                    v = 0x09;
                    break;

                case "[C]":
                    v = 0x0a;
                    break;

                case "[X]":
                    v = 0x0b;
                    break;

                case "[Y]":
                    v = 0x0c;
                    break;

                case "[Z]":
                    v = 0x0d;
                    break;

                case "[I]":
                    v = 0x0e;
                    break;

                case "[J]":
                    v = 0x0f;
                    break;

                case "PUSH":
                case "[--SP]":
                    v = 0x18;
                    break;

                case "POP":
                case "[SP++]":
                    v = 0x18;
                    break;

                case "PEEK":
                case "[SP]":
                    v = 0x19;
                    break;

                case "PICK":
                    v = 0x1a;
                    break;

                case "SP":
                    v = 0x1b;
                    break;

                case "PC":
                    v = 0x1c;
                    break;

                case "EX":
                    v = 0x1d;
                    break;

                default:
                    {
                        String label = "";

                        //number - if <= 0x1f then use literal 0x20 to 0x3f, else use 0x1f and next word
                        int num;
                        bool isNum = parseNum(V, out num);
                        if (isNum)
                        {
                            if (num >= -1 && num <= 0x1f)
                            {
                                v = (ushort)(num + 0x21);
                            }
                            else
                            {
                                v = 0x1f;

                                int[] ints = { num };
                                nextWord = new BitArray(ints);
                            }
                        }
                        else if (V.Length >= 2)
                        {
                            if (bracketOpenChars.Contains(V.Substring(0, 1).ToCharArray()[0]))
                            {
                                String strNum = V.Remove(0, 1);
                                strNum = strNum.Remove(strNum.Length - 1, 1).Trim();

                                int plusIndex = strNum.IndexOf("+");

                                if (plusIndex >= 0) //[number + A] - use 0x10 to 0x17 and next word
                                {
                                    String left = strNum.Remove(plusIndex, strNum.Length - plusIndex).Trim();
                                    String right = strNum.Remove(0, plusIndex + 1).Trim();

                                    String reg = "";
                                    String labelPlus = "";

                                    if (registers.Contains(left.ToUpper()))
                                    {
                                        reg = left;
                                        strNum = right;
                                    }
                                    else if (registers.Contains(right.ToUpper()))
                                    {
                                        reg = right;
                                        strNum = left;
                                    }
                                    else
                                    {
                                        v = 0x1e;

                                        int numLeft, numRight;

                                        if (parseNum(left, out numLeft))
                                        {
                                            if (parseNum(right, out numRight))
                                            {
                                                strNum = (numLeft + numRight).ToString();
                                            }
                                            else //right is a label
                                            {
                                                strNum = right;
                                                labelPlus = left;
                                            }
                                        }
                                        else //left is a label
                                        {
                                            if (parseNum(right, out numRight))
                                            {
                                                strNum = left;
                                                labelPlus = right;
                                            }
                                            else //right is also a label
                                            {
                                                strNum = left;
                                                labelPlus = right;
                                            }
                                        }

                                    }

                                    if (reg != "")
                                    {

                                        //MessageBox.Show("reg: " + reg);

                                        switch (reg.ToUpper())
                                        {
                                            case "A":
                                                v = 0x10;
                                                break;

                                            case "B":
                                                v = 0x11;
                                                break;

                                            case "C":
                                                v = 0x12;
                                                break;

                                            case "X":
                                                v = 0x13;
                                                break;

                                            case "Y":
                                                v = 0x14;
                                                break;

                                            case "Z":
                                                v = 0x15;
                                                break;

                                            case "I":
                                                v = 0x16;
                                                break;

                                            case "J":
                                                v = 0x17;
                                                break;
                                        }

                                    }

                                    if (parseNum(strNum, out num))
                                    {
                                        int[] ints = { num };
                                        nextWord = new BitArray(ints);
                                    }
                                    else
                                    {
                                        label = strNum;

                                        nextWord = new BitArray(16);

                                        Label lab = new Label(label, (ushort)(location + locationPlus + 1), line, lineStr);
                                        if (labelPlus != "")
                                        {
                                            lab.plus = labelPlus;
                                        }
                                        labelsTo.Add(lab);
                                    }
                                }
                                else //[number] - use 0x1e and next word
                                {
                                    v = 0x1e;

                                    if (parseNum(strNum, out num))
                                    {
                                        int[] ints = { num };
                                        nextWord = new BitArray(ints);
                                    }
                                    else
                                    {
                                        label = strNum.Trim();


                                        nextWord = new BitArray(16);

                                        Label lab = new Label(label, (ushort)(location + locationPlus + 1), line, lineStr);
                                        labelsTo.Add(lab);
                                    }
                                }
                            }
                            else if (V[0] == '"' && V[V.Length - 1] == '"') //quote
                            {
                                int n = (int)V.Substring(1, V.Length - 2)[0];

                                if (n >= -1 && n <= 0x1e)
                                {
                                    v = (ushort)(num + 0x21);
                                }
                                else
                                {
                                    v = 0x1f;

                                    int[] ints = { n };
                                    nextWord = new BitArray(ints);
                                }
                            }
                            else //label - find label and convert it to number then do the same as with normal numbers
                            {
                                label = V;

                                v = 0x1f;

                                nextWord = new BitArray(16);

                                Label lab = new Label(label, (ushort)(location + locationPlus + 1), line, lineStr);
                                labelsTo.Add(lab);
                            }
                        }

                        break;
                    }
            }

            //MessageBox.Show(V + " " + v.ToString());


            int[] intsv = { v };
            BitArray r = new BitArray(intsv);
            r.Length = 6;


            if (nextWord != null)
            {
                locationPlus++;
            }

            //MessageBox.Show("v: " + v);

            return r;
        }

        bool parseNum(String str, out int num)
        {
            //MessageBox.Show(str);

            bool ok = false;
            num = -1;

            int indexStart = str.ToLower().IndexOf("0x");
            int indexEnd = 0;

            //MessageBox.Show("index: " + indexStart.ToString());

            while (indexStart >= 0)
            {
                for (int i = indexStart+2; i < str.Length; i++)
                {
                    char ch = str[i];
                    //MessageBox.Show("Ch: " + ch);

                    int outt;

                    bool go = false;

                    if (!hexChars.Contains(ch.ToString().ToUpper()[0]))
                    {
                        indexEnd = i;
                        go = true;
                    }
                    else if (i >= str.Length-1)
                    {
                        indexEnd = i+1;
                        go = true;
                    }


                    if (go)
                    {
                        

                        String nStr = str.Substring(indexStart+2, indexEnd-indexStart-2).Trim();

                        if (nStr != "")
                        {

                            //MessageBox.Show("nstr: " + nStr);

                            int n = Convert.ToInt32(nStr, 16);

                            str = str.Remove(indexStart, indexEnd - indexStart);
                            str = str.Insert(indexStart, n.ToString());

                        }

                        break;
                    }
                }


                indexStart = str.ToLower().IndexOf("0x");
            }

            //MessageBox.Show(str);

            if (str == "")
            {
                ok = false;
            }
            else
            {
                if (int.TryParse(str, out num))
                {
                    ok = true;
                }
                else
                {

                    Expression e = new Expression(str);

                    try
                    {
                        num = (int)e.Evaluate();
                        ok = true;
                    }
                    catch (Exception ex)
                    {
                        ok = false;
                    }
                }
            }

            /*
            if (str.Length >= 2)
            {
                if (str.ToLower().Substring(0, 2) == "0x")
                {
                    try
                    {
                        num = Convert.ToInt32(str.Remove(0, 2), 16);
                        ok = true;
                    }
                    catch (Exception ex)
                    {
                        error("hex error: " + str);
                    }
                }
                else if (int.TryParse(str, out num))
                {
                    ok = true;
                }
            }
            else if (int.TryParse(str, out num))
            {
                ok = true;
            }*/

            return ok;
        }

        void computeLabels()
        {

            //MessageBox.Show(labels.Count.ToString());
            
            foreach (Label label in labels)
            {
                if (registers.Contains(label.name.ToUpper()))
                {
                    error("Label name reserved: " + label.nameReal, label.line, label.lineStr);
                    break;
                }

                foreach (Label labelTo in labelsTo)
                {
                    if (label.name == labelTo.name)
                    {
                        labelTo.labelLocation = label.location;

                        labelTo.found = true;
                    }
                }
            }

            foreach (Label labelTo in labelsTo)
            {
                if (!labelTo.found)
                {
                    error("Label not found: " + labelTo.nameReal, labelTo.line, labelTo.lineStr);
                    break;
                }

                if (labelTo.plus != "")
                {
                    int num;
                    if (parseNum(labelTo.plus, out num))
                    {
                        labelTo.labelLocation += (ushort)num;
                    }
                    else
                    {
                        bool found = false;
                        foreach (Label labelTo2 in labelsTo)
                        {
                            if (labelTo.plus == labelTo2.name)
                            {
                                labelTo.labelLocation += labelTo2.labelLocation;
                                found = true;
                                break;
                            }
                        }
                        if (!found)
                        {
                            error("Couldn't find the label (" + labelTo.plus + ") that was being added to the label: " + labelTo.nameReal, labelTo.line, labelTo.lineStr);
                            break;
                        }
                    }
                }
                
                //MessageBox.Show(labelTo.location.ToString() + " " + labelTo.labelLocation.ToString());

                memory[labelTo.location] = labelTo.labelLocation;

                foreach(Code c in codeList)
                {
                    if (c.location == labelTo.location)
                    {
                        c.word = labelTo.labelLocation;
                        break;
                    }
                }
            }
        }

        void error(String str)
        {
            error(str, line, lineStr);
        }

        void error(String str, int line, String lineStr)
        {
            MessageBox.Show(str + Environment.NewLine + "(line " + (line+1).ToString() + ") " + lineStr);
            Error = true;
        }

        void memoryClear()
        {
            for (int i = 0; i < memory.Length; i++)
            {
                memory[i] = 0x0;
            }
        }

        void memoryDump()
        {
            memoryDump(0, memory.Length);
        }

        void memoryDump(int start, int length)
        {
            richTextBoxMemory.Text = "";
            for (int i = start; i < start+length; i++)
            {
                //MessageBox.Show("0x" + memory[i].ToString("x4"));
                if (i % 8 == 0) richTextBoxMemory.AppendText(Environment.NewLine + (i).ToString("x4")+": "); 
                richTextBoxMemory.AppendText(memory[i].ToString("x4")+" ");

                //if (memory[i] == 0x0) break;
            }
        }

        void codeDump(int editorLineCount)
        {
            setMemoryText("");

            int i = 0;
            int line = 0;
            String str = "";
            int strLine = 0;

            foreach (Code c in codeList)
            {
                for (int j = i; j < c.line; j++)
                {
                    str += Environment.NewLine;
                    strLine++;
                }

                i = c.line;

                if (line != c.line)
                {
                    line = c.line;
                    str += c.location.ToString("x4") + ": ";
                }
                
                str += c.word.ToString("x4") + " ";
                
            }

            for (int j = strLine; j < editorLineCount - 1; j++)
            {
                str += Environment.NewLine;
                strLine++;
            }

            setMemoryText(str);
        }

        void writeMemory()
        {
            FileStream fs = File.Create("out.dcpu");
            BinaryWriter bw = new BinaryWriter(fs);

            for (int i = 0; i < location; i++)
            {
                bw.Write(memory[i]);
            }

            bw.Close();
            fs.Close();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            richTextBoxEditor.AddPeer(richTextBoxMemory);

            richTextBoxMemory.AddPeer(richTextBoxEditor);
        }

        private void backgroundWorkerAssemble_DoWork(object sender, System.ComponentModel.DoWorkEventArgs e)
        {
            assemble();
        }

        

        

    }

        

    
}
