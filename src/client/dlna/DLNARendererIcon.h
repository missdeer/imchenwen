#ifndef DLNARENDERERICON_H
#define DLNARENDERERICON_H

#include <QString>

// Container for DLNARenderer's Icon information
struct DLNARendererIcon
{
    // Default value
    int width=0;
    int height=0;
    QString mimetype;
    QString url;
};

#endif // DLNARENDERERICON_H
