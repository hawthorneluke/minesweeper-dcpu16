using System.Collections.Generic;
using System.Windows.Forms;
using System;
using System.Drawing;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Drawing.Drawing2D;


namespace dcpu_16Assembler
{
    partial class Form1
    {
        /// <summary>
        /// 必要なデザイナー変数です。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 使用中のリソースをすべてクリーンアップします。
        /// </summary>
        /// <param name="disposing">マネージ リソースが破棄される場合 true、破棄されない場合は false です。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows フォーム デザイナーで生成されたコード

        /// <summary>
        /// デザイナー サポートに必要なメソッドです。このメソッドの内容を
        /// コード エディターで変更しないでください。
        /// </summary>
        private void InitializeComponent()
        {
            this.buttonAssemble = new System.Windows.Forms.Button();
            this.richTextBoxMemory = new dcpu_16Assembler.ScintillaSynchronizedScroll();
            this.richTextBoxEditor = new dcpu_16Assembler.ScintillaSynchronizedScroll();
            this.backgroundWorkerAssemble = new System.ComponentModel.BackgroundWorker();
            this.progressBarAssemble = new System.Windows.Forms.ProgressBar();
            ((System.ComponentModel.ISupportInitialize)(this.richTextBoxMemory)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.richTextBoxEditor)).BeginInit();
            this.SuspendLayout();
            // 
            // buttonAssemble
            // 
            this.buttonAssemble.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonAssemble.Location = new System.Drawing.Point(12, 507);
            this.buttonAssemble.Name = "buttonAssemble";
            this.buttonAssemble.Size = new System.Drawing.Size(75, 23);
            this.buttonAssemble.TabIndex = 1;
            this.buttonAssemble.Text = "Assemble";
            this.buttonAssemble.UseVisualStyleBackColor = true;
            this.buttonAssemble.Click += new System.EventHandler(this.buttonCompile_Click);
            // 
            // richTextBoxMemory
            // 
            this.richTextBoxMemory.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.richTextBoxMemory.IsReadOnly = true;
            this.richTextBoxMemory.Location = new System.Drawing.Point(542, 12);
            this.richTextBoxMemory.Margins.Margin0.Width = 50;
            this.richTextBoxMemory.Margins.Margin1.Width = 1;
            this.richTextBoxMemory.Name = "richTextBoxMemory";
            this.richTextBoxMemory.Scrolling.HorizontalWidth = 1;
            this.richTextBoxMemory.Size = new System.Drawing.Size(238, 489);
            this.richTextBoxMemory.Styles.BraceBad.Size = 9F;
            this.richTextBoxMemory.Styles.BraceLight.Size = 9F;
            this.richTextBoxMemory.Styles.ControlChar.Size = 9F;
            this.richTextBoxMemory.Styles.Default.BackColor = System.Drawing.SystemColors.Window;
            this.richTextBoxMemory.Styles.Default.Size = 9F;
            this.richTextBoxMemory.Styles.IndentGuide.Size = 9F;
            this.richTextBoxMemory.Styles.LastPredefined.Size = 9F;
            this.richTextBoxMemory.Styles.LineNumber.Size = 9F;
            this.richTextBoxMemory.Styles.Max.Size = 9F;
            this.richTextBoxMemory.TabIndex = 2;
            // 
            // richTextBoxEditor
            // 
            this.richTextBoxEditor.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.richTextBoxEditor.Location = new System.Drawing.Point(12, 12);
            this.richTextBoxEditor.Margins.Margin0.Width = 50;
            this.richTextBoxEditor.Margins.Margin1.Width = 1;
            this.richTextBoxEditor.Name = "richTextBoxEditor";
            this.richTextBoxEditor.Scrolling.HorizontalWidth = 1;
            this.richTextBoxEditor.Size = new System.Drawing.Size(524, 489);
            this.richTextBoxEditor.Styles.BraceBad.Size = 9F;
            this.richTextBoxEditor.Styles.BraceLight.Size = 9F;
            this.richTextBoxEditor.Styles.ControlChar.Size = 9F;
            this.richTextBoxEditor.Styles.Default.BackColor = System.Drawing.SystemColors.Window;
            this.richTextBoxEditor.Styles.Default.Size = 9F;
            this.richTextBoxEditor.Styles.IndentGuide.Size = 9F;
            this.richTextBoxEditor.Styles.LastPredefined.Size = 9F;
            this.richTextBoxEditor.Styles.LineNumber.Size = 9F;
            this.richTextBoxEditor.Styles.Max.Size = 9F;
            this.richTextBoxEditor.TabIndex = 0;
            // 
            // backgroundWorkerAssemble
            // 
            this.backgroundWorkerAssemble.WorkerReportsProgress = true;
            this.backgroundWorkerAssemble.WorkerSupportsCancellation = true;
            this.backgroundWorkerAssemble.DoWork += new System.ComponentModel.DoWorkEventHandler(this.backgroundWorkerAssemble_DoWork);
            // 
            // progressBarAssemble
            // 
            this.progressBarAssemble.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.progressBarAssemble.Location = new System.Drawing.Point(93, 507);
            this.progressBarAssemble.Name = "progressBarAssemble";
            this.progressBarAssemble.Size = new System.Drawing.Size(687, 23);
            this.progressBarAssemble.TabIndex = 3;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(792, 542);
            this.Controls.Add(this.progressBarAssemble);
            this.Controls.Add(this.richTextBoxMemory);
            this.Controls.Add(this.buttonAssemble);
            this.Controls.Add(this.richTextBoxEditor);
            this.Name = "Form1";
            this.Text = "DCPU-16 Assembler";
            this.Load += new System.EventHandler(this.Form1_Load);
            ((System.ComponentModel.ISupportInitialize)(this.richTextBoxMemory)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.richTextBoxEditor)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button buttonAssemble;
        private ScintillaSynchronizedScroll richTextBoxEditor;
        private ScintillaSynchronizedScroll richTextBoxMemory;
        private BackgroundWorker backgroundWorkerAssemble;
        private ProgressBar progressBarAssemble;

    }

    
    class ScintillaSynchronizedScroll : ScintillaNET.Scintilla
    {
        public const int WM_VSCROLL = 0x115;

        List<ScintillaSynchronizedScroll> peers = new List<ScintillaSynchronizedScroll>();

        public void AddPeer(ScintillaSynchronizedScroll peer)
        {
            this.peers.Add(peer);
        }

        /*
        private void DirectWndProc(ref Message m)
        {
            //MessageBox.Show(this.Name + " " + this.Handle.ToString() + " " + m.Msg.ToString() + " " + m.WParam.ToString() + " " + m.LParam.ToString());
               
            base.WndProc(ref m);
        }
         * */

        protected override void WndProc(ref Message m)
        {
            if (m.Msg == WM_VSCROLL)
            {
                //MessageBox.Show(this.Name + " " + this.Handle.ToString() + " " + m.Msg.ToString() + " " + m.WParam.ToString() + " " + m.LParam.ToString());
                foreach (var peer in this.peers)
                {
                    //var peerMessage = Message.Create(peer.Handle, m.Msg, m.WParam, m.LParam);
                    //peer.DirectWndProc(ref peerMessage);
                    if (peer.Caret.LineNumber >= this.Lines.FirstVisible.VisibleLineNumber)
                    peer.Caret.LineNumber = this.Lines.FirstVisible.VisibleLineNumber;
                    else
                        peer.Caret.LineNumber = this.Lines.FirstVisible.VisibleLineNumber +peer.Lines.VisibleLines.Length;
                }
            }

            base.WndProc(ref m);
        }
    }

}

