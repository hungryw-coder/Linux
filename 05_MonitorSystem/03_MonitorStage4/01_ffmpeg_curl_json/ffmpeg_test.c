#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>

int main(int argc, char *argv[])
{
	printf("%d\n", avcodec_version());
    return 0;
}
