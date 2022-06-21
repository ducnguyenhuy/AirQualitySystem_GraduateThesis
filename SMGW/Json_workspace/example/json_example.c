// gcc json-parse04.c -ljson-c -o json-parse04

#include <stdio.h>
#include <json-c/json.h>

int
main(void)
{
   int temp_n;
   json_object *root, *temp;

   root = json_object_from_file("contact.json");
   if (!root)
      return 1;

   json_object *phone_numbers = json_object_object_get(root, "phoneNumbers");
   temp_n = json_object_array_length(phone_numbers);
   for (int i = 0; i < temp_n; i++)
      {
         temp = json_object_array_get_idx(phone_numbers, i);
         json_object *type = json_object_object_get(temp, "type");
         json_object *number = json_object_object_get(temp, "number");
         printf("\tPhone number %d: %s: %s\n", i, json_object_get_string(type), json_object_get_string(number));
      }

   printf("Children:\n");
   json_object *children = json_object_object_get(root, "children");
   if (json_object_get_type(children) == json_type_null)
      printf("\tNo children\n");
   else
      {
         temp_n = json_object_array_length(children);
         if (temp_n)
            for (int i = 0; i < temp_n; i++)
               {
                  temp = json_object_array_get_idx(children, i);
                  json_object *name = json_object_object_get(temp, "name");
                  printf("\t Name: %s\n", json_object_get_string(name));
                  age = json_object_object_get(temp, "age");
                  printf("\t Age: %d\n\n", json_object_get_int(age));
               }
         else
            printf("\tUnknown\n");
      }

   printf("Spouse:\n");
   json_object *spouse = json_object_object_get(root, "spouse");
   if (json_object_get_type(spouse) == json_type_null)
      printf("\tNo Spouse Known\n");
   else
      {
         json_object *name = json_object_object_get(spouse, "name");
         printf("\t Name: %s\n", json_object_get_string(name));
         age = json_object_object_get(spouse, "age");
         printf("\t Age: %d\n\n", json_object_get_int(age));
      }

   json_object_put(root);
   return 0;
}