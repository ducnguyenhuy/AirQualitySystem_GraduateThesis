#include <stdio.h>
#include <json-c/json.h>
#include <math.h>
#include <unistd.h>

#define FILE_NAME "aqi_and_concentration.json"

typedef struct
{
    /* data */
    int addr;
    float aqi;
    float pm2_5;
    float co;
} data_write_to_json_t;

int creat_file(char *file_name)
{
    json_object *root = json_object_new_object();
    if (!root)
        return 1;

    json_object *node = json_object_new_array();

    json_object_object_add(root, "node", node);

    // save json
    if (json_object_to_file_ext(FILE_NAME, root, JSON_C_TO_STRING_PRETTY))
        printf("Error: failed to save %s!!\n", FILE_NAME);
    else
        printf("%s saved.\n", FILE_NAME);
}

int edit_data(json_object *node_i, data_write_to_json_t newValue)
{
    json_object *aqi, *pm2_5, *co;
    json_object_object_add(node_i, "address", json_object_new_int(newValue.addr));
    json_object_object_add(node_i, "aqi", json_object_new_int(newValue.aqi));
    json_object_object_add(node_i, "pm2_5", json_object_new_double(newValue.pm2_5));
    json_object_object_add(node_i, "co", json_object_new_double(newValue.co));
}

int write_to_json_file(char *filename, data_write_to_json_t data)
{
    int i, lenArr;
    json_object *node_arr, *node_i, *addr;
    json_object *root = json_object_from_file(FILE_NAME);
     
    json_object_object_get_ex(root, "node", &node_arr);
    lenArr = json_object_array_length(node_arr);

    for(i = 0; i < lenArr; i++)
    {
        node_i = json_object_array_get_idx(node_arr, i);
        json_object_object_get_ex(node_i, "address", &addr);

        if(json_object_get_int(addr) == data.addr)
        {
            edit_data(node_i, data);
            /* Save file */
            if (json_object_to_file_ext(FILE_NAME, root, JSON_C_TO_STRING_PRETTY)) 
            {
                printf("Error: failed to save %s!!\n", FILE_NAME);
            }
            else
            {
                printf("%s saved.\n", FILE_NAME);
            }
            json_object_put(root);
            return 0;
        }
    }
    json_object *new_obj = json_object_new_object();
    edit_data(new_obj, data);
    json_object_array_add(node_arr, new_obj);

    /* Save file */
    if (json_object_to_file_ext(FILE_NAME, root, JSON_C_TO_STRING_PRETTY))
    {
        printf("Error: failed to save %s!!\n", FILE_NAME);
    }
    else
    {
        printf("%s saved.\n", FILE_NAME);
    }

    return 0;
}

int main(void)
{
    data_write_to_json_t tmp;
    tmp.addr = 6;
    tmp.aqi = 131;
    tmp.co = 45;
    tmp.pm2_5 = 10;

    if(access(FILE_NAME, F_OK) != 0)
    {
        creat_file(FILE_NAME);
    }

    write_to_json_file(FILE_NAME, tmp);
 
    return 0;
}