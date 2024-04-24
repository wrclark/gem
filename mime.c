#include "mime.h"
#include <string.h>

/* maybe this should be a has map */
const char *mime_type_by_ext(const char *ext) {
    if (!strcmp(ext, "gmi")) {
        return "text/gemini";
    }

    if (!strcmp(ext, "aac")) {
        return "audio/aac";
    }

    if (!strcmp(ext, "avif")) {
        return "image/avif";
    }

    if (!strcmp(ext, "avi")) {
        return "video/x-msvideo";
    }

    if (!strcmp(ext, "azw")) {
        return "application/vnd.amazon.ebook";
    }

    if (!strcmp(ext, "bin")) {
        return "application/octet-stream";
    }

    if (!strcmp(ext, "bmp")) {
        return "image/bmp";
    }

    if (!strcmp(ext, "bz")) {
        return "application/x-bzip";
    }

    if (!strcmp(ext, "bz2")) {
        return "application/x-bzip2";
    }

    if (!strcmp(ext, "css")) {
        return "text/css";
    }

    if (!strcmp(ext, "csv")) {
        return "text/csv";
    }

    if (!strcmp(ext, "doc")) {
        return "application/msword";
    }

    if (!strcmp(ext, "docx")) {
        return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    }

    if (!strcmp(ext, "epub")) {
        return "application/epub+zip";
    }

    if (!strcmp(ext, "gz")) {
        return "application/gzip";
    }

    if (!strcmp(ext, "gif")) {
        return "image/gif";
    }

    if (!strcmp(ext, "htm") || !strcmp(ext, "html")) {
        return "text/html";
    }

    if (!strcmp(ext, "jar")) {
        return "application/java-archive";
    }

    if (!strcmp(ext, "jpg") || !strcmp(ext, "jpeg")) {
        return "image/jpeg";
    }

    if (!strcmp(ext, "js")) {
        return "text/javascript";
    }

    if (!strcmp(ext, "json")) {
        return "application/json";
    }

    if (!strcmp(ext, "mid")) {
        return "audio/midi";
    }

    if (!strcmp(ext, "midi")) {
        return "audio/x-midi";
    }

    if (!strcmp(ext, "mp3")) {
        return "audio/mpeg";
    }

    if (!strcmp(ext, "mp4")) {
        return "video/mp4";
    }

    if (!strcmp(ext, "mpeg")) {
        return "video/mpeg";
    }

    if (!strcmp(ext, "png")) {
        return "image/png";
    }

    if (!strcmp(ext, "pdf")) {
        return "application/pdf";
    }

    if (!strcmp(ext, "ppt")) {
        return "application/vnd.ms-powerpoint";
    }

    if (!strcmp(ext, "pptx")) {
        return "application/vnd.openxmlformats-officedocument.presentationml.presentation";
    }

    if (!strcmp(ext, "rar")) {
        return "application/vnd.rar";
    }

    if (!strcmp(ext, "sh")) {
        return "application/x-sh";
    }

    if (!strcmp(ext, "svg")) {
        return "image/svg+xml";
    }

    if (!strcmp(ext, "tar")) {
        return "application/x-tar";
    }

    if (!strcmp(ext, "tif") || !strcmp(ext, "tiff")) {
        return "image/tiff";
    }

    if (!strcmp(ext, "ts")) {
        return "video/mp2t";
    }

    if (!strcmp(ext, "txt")) {
        return "text/plain";
    }

    if (!strcmp(ext, "wav")) {
        return "audio/wav";
    }

    if (!strcmp(ext, "weba")) {
        return "audio/webm";
    }

    if (!strcmp(ext, "webm")) {
        return "video/webm";
    }

    if (!strcmp(ext, "webp")) {
        return "image/webp";
    }

    if (!strcmp(ext, "xhtml")) {
        return "application/xhtml+xml";
    }

    if (!strcmp(ext, "xls")) {
        return "application/vnd.ms-excel";
    }

    if (!strcmp(ext, "xlsx")) {
        return "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    }

    if (!strcmp(ext, "xml")) {
        return "application/xml";
    }

    if (!strcmp(ext, "zip")) {
        return "application/zip";
    }

    if (!strcmp(ext, "7z")) {
        return "application/x-7z-compressed";
    }


    return "application/octet-stream";
}
