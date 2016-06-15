using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;

namespace WindowsFormsApplication1
{
    static class Program
    {
        /// <summary>
        /// Punto de entrada principal para la aplicación.
        /// </summary>
        [STAThread]
        static void Main()
        {
            var imageFiles = System.IO.Directory.EnumerateFiles("C:/webgl/htdocs/media/Quake");
            foreach (string currentFile in imageFiles)
            {
                string extension = currentFile.Substring(currentFile.Length - 3);
                if (extension == "jpg")
                {
                    var originalImage = (Bitmap)(Image.FromFile(currentFile));
                    int width = pow2(originalImage.Width);
                    int heigth = pow2(originalImage.Height);
                    if (width != originalImage.Width || heigth != originalImage.Height)
                    {
                        var convertedImage = new Bitmap(width, heigth, PixelFormat.Format32bppArgb);
                        using (Graphics g = Graphics.FromImage(convertedImage))
                        {
                            g.Clear(Color.Black);
                            g.DrawImage(
                                originalImage,
                                new Rectangle(0, 0, width, heigth),
                                new Rectangle(0, 0, originalImage.Width, originalImage.Height),
                                GraphicsUnit.Pixel);
                        }
                        originalImage.Dispose();
                        convertedImage.Save(currentFile);
                    }
                }
            }
            System.Windows.Forms.MessageBox.Show("listo");
        }

    

        static int pow2(int n)
        {
            if(n<=2)
                return n;
          
            int x = 1;
          while(x < n) {
            x <<= 1;
          }

          int ant_x = x;
          ant_x >>= 1;
         if(Math.Abs(ant_x-n)<Math.Abs(x-n))
             return ant_x;
         else
            return x;
        }   


    }
}
  
