#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

int main()
{
    uid_t id = getuid();
    gid_t group = getgid();
    gid_t list[100];
    int p;
    char* name = getpwuid(id)->pw_name;
    p = getgroups(100, list);
    printf("uid=%d(%s) gid=%d(%s)", id, name, group, name);
    char* gidname = getgrgid(list[p - 1])->gr_name;
    printf(",группы=%d(%s)", list[p - 1], gidname);
    for (int i = 0; i < p - 1; ++i)
    {
        char* gidname = getgrgid(list[i])->gr_name;
        printf(",%d(%s)", list[i], gidname);
    }
    printf("\n");
    return 0;
}
