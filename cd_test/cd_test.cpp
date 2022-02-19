#include <iostream>

#include <windows.h>
#include <winioctl.h>
#include <ntddcdrm.h>
#include <ntddscsi.h>
#include <stddef.h>
#include <tchar.h>  

#include <iostream>
#include <fstream> 
using namespace std;

int main()
{
    FILE *sector;
    fopen_s(&sector, "data.bin", "wb");

    HANDLE fh;
    DWORD ioctl_bytes;
    BOOL ioctl_rv;
    UCHAR buf[2352];
    struct sptd_with_sense
    {
        SCSI_PASS_THROUGH_DIRECT s;
        UCHAR sense[128];
    } sptd;

    fh = CreateFile(_T("\\\\.\\J:"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    memset(&sptd, 0, sizeof(sptd));
    sptd.s.Length = sizeof(sptd.s);
    sptd.s.DataIn = SCSI_IOCTL_DATA_IN;
    sptd.s.TimeOutValue = 30;
    sptd.s.DataBuffer = buf;
    sptd.s.DataTransferLength = sizeof(buf);
    sptd.s.SenseInfoLength = sizeof(sptd.sense);
    sptd.s.SenseInfoOffset = offsetof(struct sptd_with_sense, sense);

    for (int i = 0; i < 100; i++)
    {
        UCHAR cdb[] = { 0xBE, 0, 0, 0, 0, 0, 0, 0, 1, 0x10, 0, 0 };
        cdb[2] = (i >> 24) & 0xff;
        cdb[3] = (i >> 16) & 0xff;
        cdb[4] = (i >> 8) & 0xff;
        cdb[5] = i & 0xff;
        sptd.s.CdbLength = sizeof(cdb);
        memcpy(sptd.s.Cdb, cdb, sizeof(cdb));

        ioctl_rv = DeviceIoControl(fh, IOCTL_SCSI_PASS_THROUGH_DIRECT, &sptd, sizeof(sptd), &sptd, sizeof(sptd), &ioctl_bytes, NULL);

        fwrite(buf, sizeof(*buf), 2352, sector);
    }

    CloseHandle(fh);

    fclose(sector);

    return 0;
}
