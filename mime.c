#include <string.h>

#include "mime.h"
#include "request.h"

static const mime_t mimes[] = {
    {".gmi", "text/gemini"},
    {".aac", "audio/aac"},
    {".avif", "image/avif"},
    {".avi", "video/x-msvideo"},
    {".azw", "application/vnd.amazon.ebook"},
    {".bin", "application/octet-stream"},
    {".bmp", "image/bmp"},
    {".bz", "application/x-bzip"},
    {".bz2", "application/x-bzip2"},
    {".css", "text/css"},
    {".csv", "text/csv"},
    {".doc", "application/msword"},
    {".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
    {".epub", "application/epub+zip"},
    {".gz", "application/gzip"},
    {".gif", "image/gif"},
    {".htm", "text/html"},
    {".html", "text/html"},
    {".jar", "application/java-archive"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".js", "text/javascript"},
    {".json", "application/json"},
    {".mid", "audio/midi"},
    {".midi", "audio/x-midi"},
    {".mp3", "audio/mpeg"},
    {".mp4", "video/mp4"},
    {".mpeg", "video/mpeg"},
    {".png", "image/png"},
    {".pdf", "application/pdf"},
    {".ppt", "application/vnd.ms-powerpoint"},
    {".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
    {".rar", "application/vnd.rar"},
    {".sh", "application/x-sh"},
    {".svg", "image/svg+xml"},
    {".tar", "application/x-tar"},
    {".tif", "image/tiff"},
    {".tiff", "image/tiff"},
    {".ts", "video/mp2t"},
    {".txt", "text/plain"},
    {".wav", "audio/wav"},
    {".weba", "audio/webm"},
    {".webm", "video/webm"},
    {".webp", "image/webp"},
    {".xhtml", "application/xhtml+xml"},
    {".xls", "application/vnd.ms-excel"},
    {".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
    {".xml", "application/xml"},
    {".zip", "application/zip"},
    {".7z", "application/x-7z-compressed"}
};

static const size_t mimes_size = sizeof(mimes) / sizeof(mimes[0]);


const char *mime_type_by_ext(const char *ext) {
    size_t i;

    if (!ext) {
        return "application/octet-stream";
    }

    /* TODO: bsearch */
    for(i=0; i<mimes_size; i++) {
        if (!strcmp(ext, mimes[i].ext)) {
            return mimes[i].mime;
        }
    }

    return "application/octet-stream";
}

/* find a mime type corresponding to the file's extension */
/* NULL on error */
const char *mime_type(const char *path) {
    const char *ext;

    if (!path) {
        return NULL;
    }

    /* Find the last occurrence of '.' in the path */
    ext = strrchr(path, '.');
    
    /* If no extension found, return "application/octet-stream" */
    if (!ext || ext == path) {
        return mime_type_by_ext(NULL);
    }

    /* Pass the extension to mime_type_by_ext function */
    return mime_type_by_ext(ext);
}