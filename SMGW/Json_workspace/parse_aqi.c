#include <stdio.h>
#include <json-c/json.h>

int main(void)
{
   int len, i;

   json_object *root = json_object_from_file("aqi_and_concentration.json");
   json_object *node_addr, *addr, *node;
   json_object_object_get_ex(root, "node", &node);

   len = json_object_array_length(node);
   
   json_object *tmp;

   for(i = 0; i<len; i++)
   {
      tmp = json_object_array_get_idx(node, i);
      json_object_object_get_ex(tmp, "address", &addr);
      printf("Address: %d\n", json_object_get_int(addr));
   }
 
   json_object_put(root);
   return 0;
}
