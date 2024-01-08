# Micro XRCE-DDS Client

[![Releases](https://img.shields.io/github/release/eProsima/Micro-XRCE-DDS-Client.svg)](https://github.com/eProsima/Micro-XRCE-DDS-Client/releases)
[![License](https://img.shields.io/github/license/eProsima/Micro-XRCE-DDS-Client.svg)](https://github.com/eProsima/Micro-XRCE-DDS-Client/blob/master/LICENSE)
[![Issues](https://img.shields.io/github/issues/eProsima/Micro-XRCE-DDS-Client.svg)](https://github.com/eProsima/Micro-XRCE-DDS-Client/issues)
[![Forks](https://img.shields.io/github/forks/eProsima/Micro-XRCE-DDS-Client.svg)](https://github.com/eProsima/Micro-XRCE-DDS-Client/network/members)
[![Stars](https://img.shields.io/github/stars/eProsima/Micro-XRCE-DDS-Client.svg)](https://github.com/eProsima/Micro-XRCE-DDS-Client/stargazers)
[![Read the Docs](https://img.shields.io/readthedocs/micro-xrce-dds?style=flat)](https://micro-xrce-dds.docs.eprosima.com/en/latest/)
[![Twitter Follow](https://img.shields.io/twitter/follow/eprosima?style=social)](https://twitter.com/EProsima)

[![Docker Build Status](https://img.shields.io/docker/cloud/build/eprosima/micro-xrce-dds-client)](https://hub.docker.com/r/eprosima/micro-xrce-dds-client/)

work with https://github.com/coolwaterld/Micro-XRCE-DDS-Agent

## Quick Start

mkdir build

cd build

cmake .. -DUCLIENT_BUILD_EXAMPLES=ON

make

cd examples/BinaryEntityCreation/

./BinaryEntityCreation 127.0.0.1 2018

## What Changed

modified:   examples/BinaryEntityCreation/CMakeLists.txt
```cmake
    add_executable(${PROJECT_NAME} main.c agv.c)
```
modified:   examples/BinaryEntityCreation/main.c
```cpp
#include "agv.h"

void on_topic(
        uxrSession* session,
        uxrObjectId object_id,
        uint16_t request_id,
        uxrStreamId stream_id,
        struct ucdrBuffer* ub,
        uint16_t length,
        void* args)
{
    (void) session; (void) object_id; (void) request_id; (void) stream_id; (void) length; (void) args;

    Distance topic;
    Distance_deserialize_topic(ub, &topic);

    printf("Received angle:%d,distance:%f \n",topic.angle,topic.distance);
}


    uxrObjectId topic_id = uxr_object_id(0x01, UXR_TOPIC_ID);
    uint16_t topic_req_distance = uxr_buffer_create_topic_bin_key(&session, reliable_out, topic_id, participant_id, "TagValue_Distance",
                    "Distance", UXR_REPLACE,true);

    uxrObjectId publisher_id = uxr_object_id(0x01, UXR_PUBLISHER_ID);
    uint16_t publisher_req = uxr_buffer_create_publisher_bin_partition(&session, reliable_out, publisher_id, participant_id,
                    UXR_REPLACE,"AGV1");
    //UXR_DURABILITY_TRANSIENT_LOCAL //by ld
    uxrObjectId datawriter_id = uxr_object_id(0x01, UXR_DATAWRITER_ID);
    uxrQoS_t qos = {
        .reliability = UXR_RELIABILITY_RELIABLE, .durability = UXR_DURABILITY_VOLATILE,
        .history = UXR_HISTORY_KEEP_LAST, .depth = 0
    };
    uint16_t datawriter_req = uxr_buffer_create_datawriter_bin(&session, reliable_out, datawriter_id, publisher_id,
                    topic_id, qos, UXR_REPLACE);

    uxrObjectId subscriber_id = uxr_object_id(0x01, UXR_SUBSCRIBER_ID);
    uint16_t subscriber_req = uxr_buffer_create_subscriber_bin_partition(&session, reliable_out, subscriber_id, participant_id,
                    UXR_REPLACE,"AGV1");

    uxrObjectId datareader_id = uxr_object_id(0x01, UXR_DATAREADER_ID);
    uint16_t datareader_req = uxr_buffer_create_datareader_bin(&session, reliable_out, datareader_id, subscriber_id,
                    topic_id, qos, UXR_REPLACE);

    // Send create entities message and wait its status
    uint16_t requests[] = {
        participant_req,topic_req_distance ,  publisher_req, datawriter_req, subscriber_req, datareader_req
    };

    while (connected && count < max_topics)
    {
        ucdrBuffer ub;
        // distance
        Distance topic = {"distance","AGV1",1,{1,2},20,30.2};
        uint32_t topic_size = Distance_size_of_topic(&topic, 0);
        uxr_prepare_output_stream(&session, reliable_out, datawriter_id, &ub, topic_size);
        Distance_serialize_topic(&ub, &topic);
        connected = uxr_run_session_time(&session, 1000);
    }

```
modified:   src/c/core/session/create_entities_bin.c 
```cpp
uint16_t uxr_buffer_create_topic_bin_key(
        uxrSession* session,
        uxrStreamId stream_id,
        uxrObjectId object_id,
        uxrObjectId participant_id,
        const char* topic_name,
        const char* type_name,
        uint8_t mode,
        bool with_key)
{
    CREATE_Payload payload;
    payload.object_representation.kind = DDS_XRCE_OBJK_TOPIC;
    uxr_object_id_to_raw(participant_id, payload.object_representation._.topic.participant_id.data);
    payload.object_representation._.topic.base.representation.format = DDS_XRCE_REPRESENTATION_IN_BINARY;

    OBJK_Topic_Binary topic;
    topic.topic_name = (char*) topic_name;
    topic.optional_type_name = true;
    topic.type_name = (char*) type_name;
    topic.optional_type_reference = true;
    if(with_key)      
        topic.type_reference = "WITH_KEY";
    else
        topic.type_reference = "NO_KEY";
       

    ucdrBuffer ub;
    ucdr_init_buffer(&ub, payload.object_representation._.topic.base.representation._.binary_representation.data,
            UXR_BINARY_SEQUENCE_MAX);
    uxr_serialize_OBJK_Topic_Binary(&ub, &topic);
    payload.object_representation._.topic.base.representation._.binary_representation.size = (uint32_t) ub.offset;

    UXR_ADD_SHARED_MEMORY_ENTITY_BIN(session, object_id, &topic);

    return uxr_common_create_entity(session, stream_id, object_id, (uint16_t) ub.offset, mode, &payload);
}


uint16_t uxr_buffer_create_publisher_bin_partition(
        uxrSession* session,
        uxrStreamId stream_id,
        uxrObjectId object_id,
        uxrObjectId participant_id,
        uint8_t mode,
        char*  partition)
{
    CREATE_Payload payload;
    payload.object_representation.kind = DDS_XRCE_OBJK_PUBLISHER;
    uxr_object_id_to_raw(participant_id, payload.object_representation._.publisher.participant_id.data);
    payload.object_representation._.publisher.base.representation.format = DDS_XRCE_REPRESENTATION_IN_BINARY;

    OBJK_Publisher_Binary publisher;
    publisher.optional_publisher_name = false;
    publisher.optional_qos = false;
    //lidong
    publisher.optional_qos = true;
    OBJK_Publisher_Binary_Qos pqos;
    StringSequence_t ss = {1,{partition}};
    pqos.optional_partitions = true;
    pqos.partitions = ss;
    BinarySequence_t bs = {0,NULL};
    pqos.optional_group_data = true;
    pqos.group_data = bs;
    publisher.qos = pqos;
    //lidong
    ucdrBuffer ub;
    ucdr_init_buffer(&ub, payload.object_representation._.publisher.base.representation._.binary_representation.data,
            UXR_BINARY_SEQUENCE_MAX);
    uxr_serialize_OBJK_Publisher_Binary(&ub, &publisher);
    payload.object_representation._.publisher.base.representation._.binary_representation.size = (uint32_t) ub.offset;

    return uxr_common_create_entity(session, stream_id, object_id, (uint16_t) ub.offset, mode, &payload);
}
uint16_t uxr_buffer_create_subscriber_bin_partition(
        uxrSession* session,
        uxrStreamId stream_id,
        uxrObjectId object_id,
        uxrObjectId participant_id,
        uint8_t mode,
        char * partition)
{
    CREATE_Payload payload;
    payload.object_representation.kind = UXR_SUBSCRIBER_ID;
    uxr_object_id_to_raw(participant_id, payload.object_representation._.subscriber.participant_id.data);
    payload.object_representation._.subscriber.base.representation.format = DDS_XRCE_REPRESENTATION_IN_BINARY;

    OBJK_Subscriber_Binary subscriber;
    subscriber.optional_subscriber_name = false;
    subscriber.optional_qos = false;
    //by lidong
    subscriber.optional_qos = true;
    OBJK_Subscriber_Binary_Qos sqos;
    StringSequence_t ss = {1,{partition}};
    sqos.optional_partitions = true;
    sqos.partitions = ss;
    BinarySequence_t bs = {0,NULL};
    sqos.optional_group_data = true;
    sqos.group_data = bs;
    subscriber.qos = sqos;
    //by lidong
    ucdrBuffer ub;
    ucdr_init_buffer(&ub, payload.object_representation._.subscriber.base.representation._.binary_representation.data,
            UXR_BINARY_SEQUENCE_MAX);
    uxr_serialize_OBJK_Subscriber_Binary(&ub, &subscriber);
    payload.object_representation._.subscriber.base.representation._.binary_representation.size = (uint32_t) ub.offset;

    return uxr_common_create_entity(session, stream_id, object_id, (uint16_t) ub.offset, mode, &payload);
}
```
modified:   include/uxr/client/core/session/create_entities_bin.h
```
UXRDLLAPI uint16_t uxr_buffer_create_topic_bin_key(
        uxrSession* session,
        uxrStreamId stream_id,
        uxrObjectId object_id,
        uxrObjectId participant_id,
        const char* topic_name,
        const char* type_name,
        uint8_t mode,
        bool key);
UXRDLLAPI uint16_t uxr_buffer_create_publisher_bin_partition(
        uxrSession* session,
        uxrStreamId stream_id,
        uxrObjectId object_id,
        uxrObjectId participant_id,
        uint8_t mode,
        char * partition);
UXRDLLAPI uint16_t uxr_buffer_create_subscriber_bin_partition(
        uxrSession* session,
        uxrStreamId stream_id,
        uxrObjectId object_id,
        uxrObjectId participant_id,
        uint8_t mode,
        char * partition);
```
With examples/BinaryEntityCreation/agv.idl Using Micro-XRCE-DDS-Gen to generate 

 - examples/BinaryEntityCreation/agv.c
 - examples/BinaryEntityCreation/agv.h




<a href="http://www.eprosima.com"><img src="https://encrypted-tbn3.gstatic.com/images?q=tbn:ANd9GcSd0PDlVz1U_7MgdTe0FRIWD0Jc9_YH-gGi0ZpLkr-qgCI6ZEoJZ5GBqQ" align="left" hspace="8" vspace="2" width="100" height="100" ></a>

*eProsima Micro XRCE-DDS* is a library implementing the [DDS-XRCE protocol](https://www.omg.org/spec/DDS-XRCE/About-DDS-XRCE/) as defined and maintained by the OMG, whose aim is to allow resource constrained devices such as microcontrollers to communicate with the [DDS](https://www.omg.org/spec/DDS/About-DDS/>) world as any other DDS actor would do.
It follows a client/server paradigm and is composed by two libraries, the *Micro XRCE-DDS Client* and the *Micro XRCE-DDS Agent*. The *Micro XRCE-DDS Clients* are lightweight entities meant to be compiled on e**X**tremely **R**esource **C**onstrained **E**nvironments, while the *Micro XRCE-DDS Agent* is a broker which bridges the *Clients* with the DDS world.

<p align="center"> <img src="https://github.com/eProsima/Micro-XRCE-DDS-Client/blob/master/docs/General.png?raw=true" alt="General architecture" width="70%"/> </p>

The *Micro XRCE-DDS Clients* request operations to the *Agent* to publish and/or subscribe to topics in the DDS global dataspace. Remote procedure calls, as defined by the [DDS-RPC standard](https://www.omg.org/spec/DDS-RPC/About-DDS-RPC/), are also supported, allowing *Clients* to communicate in the DDS dataspace according to a request/reply paradigm.
The *Agents* process these requests and send back a response with the operation status result and with the requested data, in the case of subscribe/reply operations.
The communication in the DDS world is mediated by a dedicated `ProxyClient` in charge of creating the *DDS Entities* requested by the *Clients*, such as *Participants*, *Topics*, *Publishers*, and *Subscribers*, which can interact with the DDS Global dataspace.

<p align="center"> <img src="https://github.com/eProsima/Micro-XRCE-DDS-Client/blob/master/docs/Client.png?raw=true" alt="Client architecture" width="70%"/> </p>

*eProsima Micro XRCE-DDS* provides the user with a C API to create *Micro XRCE-DDS Clients* applications. The library can be configured at compile-time via a set of CMake flags allowing to enable or disable some profiles before compilation, and to manipulate several parameters controlling some of the library's functionalities, which in turn allow tuning the library size.

The communication between a *Micro XRCE-DDS Client* and a *Micro XRCE-DDS Agent* is achieved by means of several kinds of built-in transports: **UDPv4**, **UDPv6**, **TCPv4**, **TCPv6** and **Serial** communication. In addition, there is the possibility for the user to generate its own **Custom** transport.

## Documentation

You can access the *eProsima Micro XRCE-DDS* user documentation online, which is hosted on Read the Docs.

* [Start Page](http://micro-xrce-dds.readthedocs.io)
* [Installation manual](http://micro-xrce-dds.readthedocs.io/en/latest/installation.html)
* [User manual](http://micro-xrce-dds.readthedocs.io/en/latest/introduction.html)

## Quality Declaration

**eProsima Micro XRCE-DDS Client** claims to be in the **Quality Level 1** category based on the guidelines provided by [ROS 2](https://ros.org/reps/rep-2004.html).
See the [Quality Declaration](QUALITY.md) for more details.
## Getting Help

If you need support you can reach us by mail at `support@eProsima.com` or by phone at `+34 91 804 34 48`.
