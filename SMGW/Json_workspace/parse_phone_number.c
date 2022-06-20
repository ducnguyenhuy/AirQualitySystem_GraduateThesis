#include <stdio.h>
#include <json-c/json.h>

#define PATH_TO_DATA_NODE           "aqi_and_concentration.json"
#define PATH_TO_PHONE_NODE          "phone_vs_node.json"

typedef struct
{
    /* data */
    int addr;
    float aqi;
    float pm2_5;
    float co;
} data_of_node_t;


data_of_node_t parse_data(char *PATH_TO_DATA, int node_addr)
{
   int len, i;
   data_of_node_t node_info;

   /* Initialize address value in case of node not exist*/ 
   node_info.addr = 0;

   json_object *root = json_object_from_file(PATH_TO_DATA);
   json_object *addr, *node;
   json_object_object_get_ex(root, "node", &node);

   len = json_object_array_length(node);
   
   json_object *tmp;

   for(i = 0; i<len; i++)
   {
      tmp = json_object_array_get_idx(node, i);
      json_object_object_get_ex(tmp, "address", &addr);
      if(json_object_get_int(addr) == node_addr)
      {
          node_info.addr = node_addr;
          json_object *aqi, *pm2_5, *co;
          /* Get aqi param */
          json_object_object_get_ex(tmp, "aqi", &aqi);
          node_info.aqi = json_object_get_double(aqi);

          /* Get PM2.5 param */
          json_object_object_get_ex(tmp, "pm2_5", &pm2_5);
          node_info.pm2_5 = json_object_get_double(pm2_5);

          /* Get CO param */
          json_object_object_get_ex(tmp, "co", &co);
          node_info.co = json_object_get_double(co);

          return node_info;
      }
   }
   /* Can't find node with node_adrr */
   return node_info;
}

int main(void)
{
   int lenPair, lenNum, i, j;

   data_of_node_t data;

   json_object *root = json_object_from_file("phone_vs_node.json");
   json_object *phone_and_node;
   json_object_object_get_ex(root, "phone_and_node_addr", &phone_and_node);

   lenPair = json_object_array_length(phone_and_node);
   
   json_object *tmp;
   json_object *num_arr, *phone_arr, *num, *node_addr;

   for(i = 0; i<lenPair; i++)
   {
      tmp = json_object_array_get_idx(phone_and_node, i);
      /* Get node address */
      json_object_object_get_ex(tmp, "node_addr", &node_addr);

      data = parse_data(PATH_TO_DATA_NODE, 9);
      if(data.addr == 0)
      {
          printf("Node does't exist!\n");
          return 0;
      } else
      {
          printf("AQI: %f\n", data.aqi);
      }

      /* Get phone number array of node i*/
      json_object_object_get_ex(tmp, "phone", &phone_arr);
      
      lenNum = json_object_array_length(phone_arr);
      for(j = 0; j<lenNum; j++)
      {
          /* Number j*/
          num_arr = json_object_array_get_idx(phone_arr, j);
          json_object_object_get_ex(num_arr, "num", &num);

          printf("Phone number: %s\n", json_object_get_string(num));
      }
   }

   /* Free object */
   json_object_put(root);
   json_object_put(tmp);
   json_object_put(num_arr);
   json_object_put(node_addr);
   return 0;
}
