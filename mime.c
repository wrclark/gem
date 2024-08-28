#include <string.h>

#include "mime.h"
#include "request.h"

static const mime_t mimes[] = {
    {".gmi", "text/gemini", MIME_TYPE_TEXT},
    {".aac", "audio/aac", MIME_TYPE_AUDIO},
    {".avif", "image/avif", MIME_TYPE_IMAGE},
    {".avi", "video/x-msvideo", MIME_TYPE_VIDEO | MIME_TYPE_AUDIO},
    {".azw", "application/vnd.amazon.ebook", MIME_TYPE_OTHER},
    {".bin", "application/octet-stream", MIME_TYPE_OTHER},
    {".bmp", "image/bmp", MIME_TYPE_IMAGE},
    {".bz", "application/x-bzip", MIME_TYPE_OTHER},
    {".bz2", "application/x-bzip2", MIME_TYPE_OTHER},
    {".css", "text/css", MIME_TYPE_TEXT | MIME_TYPE_OTHER},
    {".csv", "text/csv", MIME_TYPE_TEXT | MIME_TYPE_OTHER},
    {".doc", "application/msword", MIME_TYPE_OTHER},
    {".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document", MIME_TYPE_OTHER},
    {".epub", "application/epub+zip", MIME_TYPE_OTHER},
    {".gz", "application/gzip", MIME_TYPE_OTHER},
    {".gif", "image/gif", MIME_TYPE_IMAGE},
    {".htm", "text/html", MIME_TYPE_TEXT},
    {".html", "text/html", MIME_TYPE_TEXT},
    {".jar", "application/java-archive", MIME_TYPE_OTHER},
    {".jpg", "image/jpeg", MIME_TYPE_IMAGE},
    {".jpeg", "image/jpeg", MIME_TYPE_IMAGE},
    {".js", "text/javascript", MIME_TYPE_TEXT},
    {".json", "application/json", MIME_TYPE_TEXT},
    {".mid", "audio/midi", MIME_TYPE_AUDIO},
    {".midi", "audio/x-midi", MIME_TYPE_AUDIO},
    {".mp3", "audio/mpeg", MIME_TYPE_AUDIO},
    {".mp4", "video/mp4", MIME_TYPE_VIDEO | MIME_TYPE_AUDIO},
    {".mpeg", "video/mpeg", MIME_TYPE_VIDEO | MIME_TYPE_AUDIO},
    {".png", "image/png", MIME_TYPE_IMAGE},
    {".pdf", "application/pdf", MIME_TYPE_OTHER},
    {".ppt", "application/vnd.ms-powerpoint", MIME_TYPE_OTHER},
    {".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation", MIME_TYPE_OTHER},
    {".rar", "application/vnd.rar", MIME_TYPE_OTHER},
    {".sh", "application/x-sh", MIME_TYPE_OTHER | MIME_TYPE_TEXT},
    {".svg", "image/svg+xml", MIME_TYPE_OTHER | MIME_TYPE_TEXT | MIME_TYPE_IMAGE},
    {".tar", "application/x-tar", MIME_TYPE_OTHER},
    {".tif", "image/tiff", MIME_TYPE_IMAGE},
    {".tiff", "image/tiff", MIME_TYPE_IMAGE},
    {".ts", "video/mp2t", MIME_TYPE_VIDEO | MIME_TYPE_AUDIO},
    {".txt", "text/plain", MIME_TYPE_TEXT},
    {".wav", "audio/wav", MIME_TYPE_AUDIO},
    {".weba", "audio/webm", MIME_TYPE_AUDIO},
    {".webm", "video/webm", MIME_TYPE_AUDIO | MIME_TYPE_VIDEO},
    {".webp", "image/webp", MIME_TYPE_IMAGE},
    {".xhtml", "application/xhtml+xml", MIME_TYPE_TEXT | MIME_TYPE_OTHER},
    {".xls", "application/vnd.ms-excel", MIME_TYPE_OTHER},
    {".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", MIME_TYPE_OTHER},
    {".xml", "application/xml", MIME_TYPE_OTHER | MIME_TYPE_TEXT},
    {".zip", "application/zip", MIME_TYPE_OTHER},
    {".7z", "application/x-7z-compressed", MIME_TYPE_OTHER}
};

static const size_t mimes_size = sizeof(mimes) / sizeof(mimes[0]);
const mime_t default_mime = {"", "application/octet-stream", MIME_TYPE_OTHER};

const mime_t *mime_type_by_ext(const char *ext) {
    size_t i;

    if (!ext) {
        return &default_mime;
    }

    /* TODO: bsearch */
    for(i=0; i<mimes_size; i++) {
        if (!strcmp(ext, mimes[i].ext)) {
            return &mimes[i];
        }
    }

    return &default_mime;
}

/* find a mime type corresponding to the file's extension */
/* NULL on error */
const mime_t *mime_type(const char *path) {
    const char *ext;

    if (!path) {
        return NULL;
    }

    ext = strrchr(path, '.');
    
    if (!ext || ext == path) {
        return mime_type_by_ext(NULL);
    }

    return mime_type_by_ext(ext);
}