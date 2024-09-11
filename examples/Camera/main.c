// Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <uxr/client/client.h>
#include <ucdr/microcdr.h>

#include <stdio.h> //printf
#include <string.h> //strcmp
#include <stdlib.h> //atoi
#include "flycam.h"
#include <unistd.h> // For sleep function
#include <sys/time.h>

#define STREAM_HISTORY  8
#define BUFFER_SIZE     UXR_CONFIG_UDP_TRANSPORT_MTU* STREAM_HISTORY


// uxrStreamId reliable_out = {0};
// uxrObjectId datawriter_id = {0};

typedef struct
{
    uxrSession * psession;
    uxrStreamId reliable_out;
    uxrObjectId datawriter_id;
} CallbackArgs;

int ID = 0;

struct timeval current_time, pre_time={0};

void on_topic(
        uxrSession* session,
        uxrObjectId object_id,
        uint16_t request_id,
        uxrStreamId stream_id,
        struct ucdrBuffer* ub,
        uint16_t length,
        void* args
)
{
    // reponse "CAM" and pub "UPDATE" 
    
    CAM sub_topic;
    CAM_deserialize_topic(ub, &sub_topic);

    if(sub_topic.id == ID)
    {
        gettimeofday(&current_time, NULL);
        double time_taken = (current_time.tv_sec - pre_time.tv_sec) + (current_time.tv_usec - pre_time.tv_usec) / 1e6;
        if(pre_time.tv_sec!=0)
        {
            printf("Triggered id:%d, period :%.2f\n",sub_topic.id,time_taken);
        }
        
        pre_time = current_time;
        ucdrBuffer pub_ub;
        UPDATE pub_topic = {ID};
        uint32_t topic_size = UPDATE_size_of_topic(&pub_topic, 0);
        CallbackArgs* callback_args = (CallbackArgs*)args;
        uxr_prepare_output_stream(callback_args->psession, callback_args->reliable_out, callback_args->datawriter_id, &pub_ub, topic_size);
        UPDATE_serialize_topic(&pub_ub, &pub_topic);
    }
    // else{
    //     printf("Received id:%d\n",sub_topic.id);
    // }
}

    


int main(
        int args,
        char** argv)
{
    // CLI
    if (4 > args || 0 == atoi(argv[2]))
    {
        printf("usage: program [-h | --help] | ip port [<max_topics>]\n");
        return 0;
    }

    char* ip = argv[1];
    char* port = argv[2];

    ID =  atoi(argv[3]);

    // uint32_t max_topics = (args == 4) ? (uint32_t)atoi(argv[3]) : UINT32_MAX;

    // Transport
    uxrUDPTransport transport;
    if (!uxr_init_udp_transport(&transport, UXR_IPv4, ip, port))
    {
        printf("Error at create transport.\n");
        return 1;
    }

    // Session
    uxrSession session;
    uxr_init_session(&session, &transport.comm, 0xAAAABBBB+ID);
    // uxr_set_topic_callback(&session, on_topic, NULL);
    if (!uxr_create_session(&session))
    {
        printf("Error at create session.\n");
        return 1;
    }



    // Streams
    uint8_t output_reliable_stream_buffer[BUFFER_SIZE];
    uxrStreamId reliable_out = uxr_create_output_reliable_stream(&session, output_reliable_stream_buffer, BUFFER_SIZE,STREAM_HISTORY);

    uint8_t input_reliable_stream_buffer[BUFFER_SIZE];
    uxrStreamId reliable_in = uxr_create_input_reliable_stream(&session, input_reliable_stream_buffer, BUFFER_SIZE,STREAM_HISTORY);

    // Create entities
    uxrObjectId participant_id = uxr_object_id(0x01, UXR_PARTICIPANT_ID);
    uint16_t participant_req = uxr_buffer_create_participant_bin(&session, reliable_out, participant_id, 0, NULL, UXR_REPLACE);

    uxrObjectId topic_id_update = uxr_object_id(0x01, UXR_TOPIC_ID);
    uint16_t topic_update_req = uxr_buffer_create_topic_bin_key(&session, reliable_out, topic_id_update, participant_id, "TagValue_UPDATE", "UPDATE", UXR_REPLACE,true);
    uxrObjectId topic_id_cam = uxr_object_id(0x02, UXR_TOPIC_ID);
    uint16_t topic_cam_req = uxr_buffer_create_topic_bin_key(&session, reliable_out, topic_id_cam, participant_id, "TagValue_CAM", "CAM", UXR_REPLACE,true);

    uxrObjectId publisher_id = uxr_object_id(0x01, UXR_PUBLISHER_ID);
    uint16_t publisher_req = uxr_buffer_create_publisher_bin_partition(&session, reliable_out, publisher_id, participant_id, UXR_REPLACE,"partition_flycam");


    uxrObjectId datawriter_id = uxr_object_id(0x01, UXR_DATAWRITER_ID);
    uxrQoS_t qos = {
        .reliability = UXR_RELIABILITY_RELIABLE, .durability = UXR_DURABILITY_VOLATILE,
        .history = UXR_HISTORY_KEEP_LAST, .depth = 0
    };
    uint16_t datawriter_req = uxr_buffer_create_datawriter_bin(&session, reliable_out, datawriter_id, publisher_id, topic_id_update, qos, UXR_REPLACE);

    uxrObjectId subscriber_id = uxr_object_id(0x01, UXR_SUBSCRIBER_ID);
    uint16_t subscriber_req = uxr_buffer_create_subscriber_bin_partition(&session, reliable_out, subscriber_id, participant_id,UXR_REPLACE,"partition_flycam");

    uxrObjectId datareader_id = uxr_object_id(0x01, UXR_DATAREADER_ID);
    uint16_t datareader_req = uxr_buffer_create_datareader_bin(&session, reliable_out, datareader_id, subscriber_id,topic_id_cam, qos, UXR_REPLACE);

    // Send create entities message and wait its status
    uint16_t requests[] = {
        participant_req,topic_update_req,topic_cam_req,  publisher_req, datawriter_req /*, subscriber_req, datareader_req*/
    };
    uint8_t status[sizeof(requests) / 2];
    if (!uxr_run_session_until_all_status(&session, 1000, requests, status, sizeof(status)))
    {
        printf("Error at create entities.\n");
        return 1;
    }

    // Request topics
    uxrDeliveryControl delivery_control = {0};
    delivery_control.max_samples = UXR_MAX_SAMPLES_UNLIMITED;
    uxr_buffer_request_data(&session, reliable_out, datareader_id, reliable_in, &delivery_control);

    CallbackArgs callback_args;
    callback_args.psession = &session;
    callback_args.datawriter_id = datawriter_id;
    callback_args.reliable_out = reliable_out;

    uxr_set_topic_callback(&session, on_topic, (void*)&callback_args);

    printf("Camera %d\n",ID);

    // printf("Waiting so subscriber sockets can connect...")
    sleep(1);
    //init pub "UPDATE"
    ucdrBuffer ub;
    UPDATE topic = {ID};
    uint32_t topic_size = UPDATE_size_of_topic(&topic, 0);
    uxr_prepare_output_stream(&session, reliable_out, datawriter_id, &ub, topic_size);
    UPDATE_serialize_topic(&ub, &topic);
    printf("init UPDATE %d\n",ID);
    // gettimeofday(&pre_time, NULL);

    bool connected = true;
    while (connected)
    {
        
        connected = uxr_run_session_time(&session, 1000);
    }

    // Delete resources
    uxr_delete_session(&session);
    uxr_close_udp_transport(&transport);

    return 0;
}
