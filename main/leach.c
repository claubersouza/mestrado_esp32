
#include <stdio.h>
#include <string.h>
#include "include/leach.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "random.h"
#include "include/scan_wifi.h"
#include "esp_log.h"
#include "include/udp_client.h"
#include "include/simple_wifi.h"
#include "esp_wifi.h"
const float p = 0.05;
int rnd = 0;
int CLheads=0;

struct Node {
    int id; // sensor's ID number
    int E;      //nodes energy levels (initially set to be equal to "Eo"
    int role;   // node acts as normal if the value is '0', if elected as a cluster head it  gets the value '1' (initially all nodes are normal)
    int cluster;	// the cluster which a node belongs to
    int cond;	// States the current condition of the node. when the node is operational its value is =1 and when dead =0
    int rop;	// number of rounds node was operational
    int rleft;  // rounds left for node to become available for Cluster Head election
    int dtch;	// nodes distance from the cluster head of the cluster in which he belongs
    int dts;    // nodes distance from the sink
    int tel;	// states how many times the node was elected as a Cluster Head
    int rn;     // round node got elected as cluster head
    int chid   // node ID of the cluster head which the "i" normal node belongs to
};

// Declare node of type Node
struct Node node; 


void initStruct() {
    node.id = 0;
    node.E = 100; // Energy initial is 100%
    node.role = 0;
    node.cond = 1;
    node.rleft = 0;
    node.dtch = 0;
    node.dts = 0; //Set RSSI
    node.tel = 0;
    node.rn  = 0;
}

float  generateRandom() {

        while (1) {
    	// wait 5 second
		vTaskDelay(1000 / portTICK_RATE_MS);
        
        // get a new random number and print it
		float generate = (float) random(1,9)/9.0;
        if ( generate > 0)
            generate = generate / 10;
            ESP_LOGI("LEACH RANDOM", "%f",generate);
            return generate;
        }

        return 0;
        //vTaskDelete(NULL);

}

void sendMsg(int rssi) {
    initUdp(rssi);
}

float calculeThreshold() {
    return (p/(1-p*(rnd%1/p)));
}

float reelection() {
    float tleft= rnd%1/p;

    return tleft;
}

void initLeach(void *pvParameters) {
    ESP_LOGI("Chegou", "Funcionando");
    initStruct();
    electionCH();
    //printf("chegou");
    vTaskDelete(NULL);
}


void electionCH() {
    
    ESP_LOGI("Chegou", "Vai eleger");
    if (node.rleft > 0 ) {
        node.rleft = node.rleft - 1;
    }
   
    //Check if Energy is > 0 and available for Cluster Head election
    if(node.E > 0 && node.rleft == 0) {
        //if (generateRandom() > calculeThreshold() ) {      
            node.dts = getCH("Clai2.4"); 
            node.role = 1;
            node.rn = rnd;
            node.tel = node.tel + 1;
            node.rleft = 1/p-reelection();
            node.cluster = CLheads +1;
           
            ESP_LOGI("Valor", "Valor RSSI Clai:%d",node.dts);
                   
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            sendMsg(node.dts);
        //}
    }

}