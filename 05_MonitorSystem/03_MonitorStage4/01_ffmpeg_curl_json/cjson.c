#include <stdio.h>
#include <cjson/cJSON.h>

int main() {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "name", "Jackie");
    cJSON_AddNumberToObject(root, "age", 40);
    
    char *json_str = cJSON_Print(root);
    printf("%s\n", json_str);
    
    cJSON_free(json_str);
    cJSON_Delete(root);
    
    return 0;
}
