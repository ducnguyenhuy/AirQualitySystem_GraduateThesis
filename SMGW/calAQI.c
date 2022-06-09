#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define NUMBER_SAMPLE_PER_HOUR  10

int pm25Index = 0, coIndex = 0, buff12hIndex = 0;


float I[8] = {0, 50, 100, 150, 200, 300, 400, 500};
float BP_CO[8] = {0, 10, 30, 45, 60, 90, 120, 150};
float BP_PM25[8] = {0, 25, 50, 80, 150, 250, 350, 500};

/* PM 2.5 concentration struct */
typedef struct
{
    float buff_12_h[12];
    float pm_data_now[NUMBER_SAMPLE_PER_HOUR];
} pm_2_5_t;

/* CO concentration struct */
typedef struct
{
    float co_data_now[NUMBER_SAMPLE_PER_HOUR];
} co_t;

/* store data to calculate AQI */
typedef struct
{
    pm_2_5_t valPM25;
    co_t     valCO;
} data_of_node_t;


/* Calculate mean of co */
float mean_of_co(data_of_node_t *data_node)
{
    float sum = 0;
    int i;
    for(i = 0; i < NUMBER_SAMPLE_PER_HOUR; i++)
    {
        sum += data_node->valCO.co_data_now[i];
    }
    return sum/NUMBER_SAMPLE_PER_HOUR;
}

/* Calculate mean of pm2.5 */
float mean_of_pm25(data_of_node_t *data_node)
{
    float sum = 0;
    int i;
    for(i = 0; i < NUMBER_SAMPLE_PER_HOUR; i++)
    {
        sum += data_node->valPM25.pm_data_now[i];
    }
    return sum/NUMBER_SAMPLE_PER_HOUR;
}

void update_data(data_of_node_t *data_node, float valPm25, float valCo)
{
    int i;
    data_node->valPM25.pm_data_now[pm25Index] = valPm25;
    data_node->valCO.co_data_now[coIndex] = valCo;

    pm25Index++;
    coIndex++;
    

    if(pm25Index == NUMBER_SAMPLE_PER_HOUR)
    {
        if(buff12hIndex == 11)
        {
            for(i = 0; i < 12; i++)
            {
                data_node->valPM25.buff_12_h[i] = data_node->valPM25.buff_12_h[i+1];
            }
            data_node->valPM25.buff_12_h[11] = mean_of_pm25(data_node);
        }
        else
        {
            data_node->valPM25.buff_12_h[buff12hIndex] = mean_of_pm25(data_node);
            buff12hIndex++;
        }
        pm25Index = 0;
    }
    
    if(coIndex == NUMBER_SAMPLE_PER_HOUR)
        coIndex = 0;
}

float cal_aqi_co(data_of_node_t *data_node)
{
    float co_mean;
    co_mean = mean_of_co(data_node);
    
    int i, j;
    for(j = 0; j < 8; j++)
    {
        if(co_mean > BP_CO[j])
        {
            i = j;
        }
    }
    
    float aqi = (( (I[i+1] - I[i]) / (BP_CO[i+1] - BP_CO[i]) ) * (co_mean - BP_CO[i]) + I[i]);

    if(i == 7)
    {
        return I[7];
    } 
    else
    {
        return aqi;
    }
}

float cal_aqi_pm25(data_of_node_t *data_node)
{
    float weightIndex;
    float Cmax, Cmin;
    float nowCast = 0;
    float sumOfThree = 0;
    float up = 0, down = 0;
    if(buff12hIndex != 11)
    {
        printf("Can't not calculate AQI of PM25");
        return -1;
    }

    Cmax = data_node->valPM25.buff_12_h[0];
    Cmin = data_node->valPM25.buff_12_h[0];

    for(int i = 0; i < 12; i ++)
    {
        if(Cmax < data_node->valPM25.buff_12_h[i])
        {
            Cmax = data_node->valPM25.buff_12_h[i];
        }
        if(Cmin > data_node->valPM25.buff_12_h[i])
        {
            Cmin = data_node->valPM25.buff_12_h[i];
        }
    }
    
    weightIndex = Cmin/Cmax;
    if(weightIndex < 0.5)
    {
        weightIndex = 0.5;
        for(int i = 0; i < 12; i++)
        {
            nowCast += pow(0.5, (i+1))*(data_node->valPM25.buff_12_h[i]);
        }
    }
    else {
        for(int i = 0; i < 12; i++)
        {
            up += pow(weightIndex, i)*(data_node->valPM25.buff_12_h[i]);
            down += pow(weightIndex, i);
        }
        nowCast = up/down;
    }
    
    sumOfThree = data_node->valPM25.buff_12_h[0] + data_node->valPM25.buff_12_h[1] + data_node->valPM25.buff_12_h[2];

    if(sumOfThree == data_node->valPM25.buff_12_h[0] || sumOfThree == data_node->valPM25.buff_12_h[1] || sumOfThree == data_node->valPM25.buff_12_h[2])
    {
        printf("Can't calculate AQI \n");
        return -1;
    }

    int j;
    for(int i = 0; i < 8; i++)
    {
        if( nowCast > BP_PM25[i] )
        {
            j = i;
        }
    }
    float aqi = (( (I[j+1] - I[j]) / (BP_PM25[j+1] - BP_PM25[j]) ) * (nowCast - BP_PM25[j]) + I[j]);
    if( j == 7)
    {
        return I[j];
    }
    else {
        return aqi;
    }
}

int main()
{
    data_of_node_t *data_node, data;
    data_node = &data;

    for(int i = 0; i < 3*(NUMBER_SAMPLE_PER_HOUR); i++)
    {
        update_data(data_node, i, i);
    }
    float mean;

    for(int i = 0; i < NUMBER_SAMPLE_PER_HOUR; i++)
    {
        printf("%f\n", data_node->valPM25.pm_data_now[i]);
    }
    printf("%f\n", data_node->valPM25.buff_12_h[0]);
    printf("%f\n", data_node->valPM25.buff_12_h[1]);
    printf("%f\n", data_node->valPM25.buff_12_h[2]);

    return 0;
}