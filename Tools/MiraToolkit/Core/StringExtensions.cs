using System.IO;

namespace MiraToolkit.Core
{
    public static class StringExtensions
    {
        /// <summary>
        /// Returns a sanitized file path string.
        /// </summary>
        /// <param name="p_Path">Input path</param>
        /// <returns>Sanitized string</returns>
        public static string GetNormalizedPath(this string p_Path)
        {
            return "/" + p_Path
                    .TrimStart(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar)
                    .TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar)
                    .Replace("\\", "/").Trim().ToLower();
        }

        /// <summary>
        /// Returns a sanitized file path string while retaining letter casing.
        /// </summary>
        /// <param name="p_Path">Input path</param>
        /// <returns>Sanitized string</returns>
        public static string GetNormalizedPathRetainCase(this string p_Path)
        {
            return "/" + p_Path
                    .TrimStart(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar)
                    .TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar)
                    .Replace("\\", "/").Trim();
        }
    }
}
