#pragma once
#include <string>
#include <vector>
#include <cstring>
#include <format>
#include <tuple>


class WSJTX_Packet {
public:
    WSJTX_Packet(const std::vector<uint8_t> pkt, size_t idx)
        : index(idx), packet(pkt), MagicNumber(0), SchemaVersion(0), PacketType(0), ClientID("") {}

    std::string readutf8() {
        int32_t strLength = getInt32();
        if (strLength > 0) {
            std::string stringRead(packet.begin() + index, packet.begin() + index + strLength);
            index += strLength;
            return stringRead;
        }
        else {
            return "";
        }
    }

    std::tuple<int64_t, uint32_t, uint8_t, int32_t> getDateTime() {
        int32_t TimeOffset = 0;
        int64_t DateOff = getLongLong();
        uint32_t TimeOff = getuInt32();
        uint8_t TimeSpec = getByte();
        if (TimeSpec == 2) {
            TimeOffset = getInt32();
        }
        return std::make_tuple(DateOff, TimeOff, TimeSpec, TimeOffset);
    }

    uint8_t getByte() {
        uint8_t data;
        std::memcpy(&data, packet.data() + index, sizeof(data));
        index += sizeof(data);
        return data;
    }

    bool getBool() {
        bool data;
        std::memcpy(&data, packet.data() + index, sizeof(data));
        index += sizeof(data);
        return data;
    }

    int32_t getInt32() {
        int32_t data;
        char rev[sizeof(data)];
        for (int i = 0; i < sizeof(data); i++)
        {
            rev[sizeof(data) - 1 - i] = packet[i + index];
        }
        std::memcpy(&data, &rev[0], sizeof(data));
        index += sizeof(data);
        return data;
    }

    uint32_t getuInt32() {
        uint32_t data = 0;
        char rev[sizeof(data)];
        for (int i = 0; i < sizeof(data); i++)
        {
            rev[sizeof(data) - 1 - i] = packet[i + index];
        }
        std::memcpy(&data, &rev[0], sizeof(data));
        index += sizeof(data);
        return data;
    }

    int64_t getLongLong() {
        int64_t data;
        char rev[sizeof(data)];
        for (int i = 0; i < sizeof(data); i++)
        {
            rev[sizeof(data) - 1 - i] = packet[i + index];
        }
        std::memcpy(&data, &rev[0], sizeof(data));
        index += sizeof(data);
        return data;
    }

    double getDouble() {
        double data;
        char rev[sizeof(data)];
        for (int i = 0; i < sizeof(data); i++)
        {
            rev[sizeof(data) - 1 - i] = packet[i + index];
        }
        std::memcpy(&data, &rev[0], sizeof(data));
        index += sizeof(data);
        return data;
    }

    std::string getDefaultTxMessage() {
        int32_t strLength = packet.size();
        if (strLength > 0) {
            std::string stringRead(packet.begin() + index + 24, packet.begin() + strLength);
            index += strLength;
            return stringRead;
        }

    }

    void Decode() {
        MagicNumber = getuInt32();
        SchemaVersion = getuInt32();
        PacketType = getuInt32();
        ClientID = readutf8();
    }

public:
    size_t index;
    std::vector<uint8_t> packet;
    uint32_t MagicNumber;
    uint32_t SchemaVersion;
    uint32_t PacketType;
    std::string ClientID;
};

// 00000220:           adbc cbda 0000 0002 0000 0000      ............
// 00000230: 0000 0006 5753 4a54 2d58 0000 0003 0000  ....WSJT - X......
// 00000240: 0005 312e 382e 3000 0000 0572 3831 3933  ..1.8.0....r8193

// Packet Type 0 Heartbeat
// The heartbeat  message shall be  sent on a periodic  basis every
// NetworkMessage::pulse   seconds   (see    below),   the   WSJT-X
// application  does  that  using the  MessageClient  class.   This
// message is intended to be used by servers to detect the presence
// of a  client and also  the unexpected disappearance of  a client
// and  by clients  to learn  the schema  negotiated by  the server
// after it receives  the initial heartbeat message  from a client.
// The message_aggregator reference server does just that using the
// MessageServer class. Upon  initial startup a client  must send a
// heartbeat message as soon as  is practical, this message is used
// to negotiate the maximum schema  number common to the client and
// server. Note  that the  server may  not be  able to  support the
// client's  requested maximum  schema  number, in  which case  the
// first  message received  from the  server will  specify a  lower
// schema number (never a higher one  as that is not allowed). If a
// server replies  with a lower  schema number then no  higher than
// that number shall be used for all further outgoing messages from
// either clients or the server itself.

// Note: the  "Maximum schema number"  field was introduced  at the
// same time as schema 3, therefore servers and clients must assume
// schema 2 is the highest schema number supported if the Heartbeat
// message does not contain the "Maximum schema number" field.

class WSJTX_Heartbeat : public WSJTX_Packet {
public:
    uint32_t MaximumSchema;
    std::string Version;
    std::string Revision;

    WSJTX_Heartbeat(const std::vector<uint8_t>& pkt, int idx) : WSJTX_Packet(pkt, idx) {
        MaximumSchema = 0;
    }

    void Decode()  {
        MaximumSchema = getuInt32();
        Version = readutf8();
        Revision = readutf8();
    }
};

// Packet Type 1 Status
// WSJT-X  sends this  status message  when various  internal state
// changes to allow the server to  track the relevant state of each
// client without the need for  polling commands. The current state
// changes that generate status messages are:

//      Application start up,
//      "Enable Tx" button status changes,
//      Dial frequency changes,
//      Changes to the "DX Call" field,
//      Operating mode, sub-mode or fast mode changes,
//      Transmit mode changed (in dual JT9+JT65 mode),
//      Changes to the "Rpt" spinner,
//      After an old decodes replay sequence (see Replay below),
//      When switching between Tx and Rx mode,
//      At the start and end of decoding,
//      When the Rx DF changes,
//      When the Tx DF changes,
//      When the DE call or grid changes (currently when settings are exited),
//      When the DX call or grid changes,
//      When the Tx watchdog is set or reset.

class WSJTX_Status : public WSJTX_Packet {
public:
    uint64_t Frequency;
    std::string Mode;
    std::string DXCall;
    std::string Report;
    std::string TxMode;
    bool TxEnabled;
    bool Transmitting;
    bool Decoding;
    uint32_t RxDF;
    uint32_t TxDF;
    std::string DECall;
    std::string DEgrid;
    std::string DXgrid;
    bool TxWatchdog;
    std::string Submode;
    bool Fastmode;
    std::string DefaultTXMessage;

    WSJTX_Status(const std::vector<uint8_t>& pkt, int idx) : WSJTX_Packet(pkt, idx) {
        Frequency = 0;
        TxEnabled = false;
        Transmitting = false;
        Decoding = false;
        RxDF = 0;
        TxDF = 0;
        TxWatchdog = false;
        Fastmode = false;
        DefaultTXMessage = "";
    }

    void Decode() {
        Frequency = getLongLong();
        Mode = readutf8();
        DXCall = readutf8();
        Report = readutf8();
        TxMode = readutf8();
        TxEnabled = getBool();
        Transmitting = getBool();
        Decoding = getBool();
        RxDF = getuInt32();
        TxDF = getuInt32();
        DECall = readutf8();
        DEgrid = readutf8();
        DXgrid = readutf8();
        TxWatchdog = getBool();
        Submode = readutf8();
        Fastmode = getBool();
        DefaultTXMessage = getDefaultTxMessage();
    }
};


// Packet Type 2
// The decode message is sent when  a new decode is completed, in
// this case the 'New' field is true. It is also used in response
// to  a "Replay"  message where  each  old decode  in the  "Band
// activity" window, that  has not been erased, is  sent in order
// as a one of these messages  with the 'New' field set to false.
// See  the "Replay"  message below  for details  of usage.   Low
// confidence decodes are flagged  in protocols where the decoder
// has knows that  a decode has a higher  than normal probability
// of  being  false, they  should  not  be reported  on  publicly
// accessible services  without some attached warning  or further
// validation. Off air decodes are those that result from playing
// back a .WAV file.

class WSJTX_Decode : public WSJTX_Packet {
public:
    bool New;
    uint32_t Time;
    int32_t snr;
    double DeltaTime;
    uint32_t DeltaFrequency;
    std::string Mode;
    std::string Message;
    bool LowConfidence;
    bool OffAir;

    WSJTX_Decode(const std::vector<uint8_t>& pkt, int idx) : WSJTX_Packet(pkt, idx) {
        New = false;
        Time = 0;
        snr = 0;
        DeltaTime = 0.0;
        DeltaFrequency = 0;
        LowConfidence = false;
        OffAir = false;
    }

    void Decode() {
        New = getBool();
        Time = getuInt32();
        snr = getInt32();
        DeltaTime = getDouble();
        DeltaFrequency = getuInt32();
        Mode = readutf8();
        Message = readutf8();
        LowConfidence = getBool();
        OffAir = getBool();
    }
};


// Packet Type 3
// This message is sent  when all prior "Decode"  messages in the
// "Band activity"  window have been discarded  and therefore are
// no long available for actioning  with a "Reply" message. It is
// sent when the user erases  the "Band activity" window and when
// WSJT-X  closes down  normally. The  server should  discard all
// decode messages upon receipt of this message.

class WSJTX_Erase : public WSJTX_Packet {
public:
    WSJTX_Erase(const std::vector<uint8_t>& pkt, int idx) : WSJTX_Packet(pkt, idx) {}
};


// Packet Type 4 Reply IN message to client
// In order for a server  to provide a useful cooperative service
// to WSJT-X it  is possible for it to initiate  a QSO by sending
// this message to a client. WSJT-X filters this message and only
// acts upon it  if the message exactly describes  a prior decode
// and that decode  is a CQ or QRZ message.   The action taken is
// exactly equivalent to the user  double clicking the message in
// the "Band activity" window. The  intent of this message is for
// servers to be able to provide an advanced look up of potential
// QSO partners, for example determining if they have been worked
// before  or if  working them  may advance  some objective  like
// award progress.  The  intention is not to  provide a secondary
// user  interface for  WSJT-X,  it is  expected  that after  QSO
// initiation the rest  of the QSO is carried  out manually using
// the normal WSJT-X user interface.
//
// The  Modifiers   field  allows  the  equivalent   of  keyboard
// modifiers to be sent "as if" those modifier keys where pressed
// while  double-clicking  the  specified  decoded  message.  The
// modifier values (hexadecimal) are as follows:
// 
//      no modifier     0x00
//      SHIFT           0x02
//      CTRL            0x04  CMD on Mac
//      ALT             0x08
//      META            0x10  Windows key on MS Windows
//      KEYPAD          0x20  Keypad or arrows
//      Group switch    0x40  X11 only

class WSJTX_Reply : public WSJTX_Packet {
public:
    WSJTX_Reply(const std::vector<uint8_t>& pkt, int idx) : WSJTX_Packet(pkt, idx) {}
};


// Packet Type 5 QSO Logged
// The  QSO logged  message is  sent  to the  server(s) when  the
// WSJT-X user accepts the "Log  QSO" dialog by clicking the "OK"
// button.

class WSJTX_Logged : public WSJTX_Packet {
public:
    WSJTX_Logged(const std::vector<uint8_t>& pkt, int idx) : WSJTX_Packet(pkt, idx) {
        DateOff = 0;
        TimeOff = 0;
        TimeOffSpec = 0;
        TimeOffOffset = 0;
        DialFrequency = 0;
        DateOn = 0;
        TimeOn = 0;
        TimeOnSpec = 0;
        TimeOnOffset = 0;
    }

    void Decode() {
        std::tuple<int64_t, uint32_t, uint8_t, int32_t> DTTuple = getDateTime();
        DateOff = std::get<0>(DTTuple);
        TimeOff = std::get<1>(DTTuple);
        TimeOffSpec = std::get<2>(DTTuple);
        TimeOffOffset = std::get<3>(DTTuple);
        DXcall = readutf8();
        DXgrid = readutf8();
        DialFrequency = getLongLong();
        Mode = readutf8();
        ReportSent = readutf8();
        ReportReceived = readutf8();
        TxPower = readutf8();
        Comments = readutf8();
        Name = readutf8();
        DTTuple = getDateTime();
        DateOn = std::get<0>(DTTuple);
        TimeOn = std::get<1>(DTTuple);
        TimeOnSpec = std::get<2>(DTTuple);
        TimeOnOffset = std::get<3>(DTTuple);
    }

private:
    int64_t DateOff;
    uint32_t TimeOff;
    uint8_t TimeOffSpec;
    int32_t TimeOffOffset;
    std::string DXcall;
    std::string DXgrid;
    int64_t DialFrequency;
    std::string Mode;
    std::string ReportSent;
    std::string ReportReceived;
    std::string TxPower;
    std::string Comments;
    std::string Name;
    int64_t DateOn;
    uint32_t TimeOn;
    uint8_t TimeOnSpec;
    int32_t TimeOnOffset;
};


// Packet Type 6 Close
//
//Close is sent by a client immediately prior to it shutting
// down gracefully.

class WSJTX_Closed : public WSJTX_Packet {
public:
    WSJTX_Closed(const std::vector<uint8_t>& pkt, int idx) : WSJTX_Packet(pkt, idx) {
        // Additional initialization specific to WSJTX_Closed
    }
};


// Packet Type 7 Replay (IN to client This is a message to be sent to the client)
//
// When a server starts it may  be useful for it to determine the
// state  of preexisting  clients. Sending  this message  to each
// client as it is discovered  will cause that client (WSJT-X) to
// send a "Decode" message for each decode currently in its "Band
// activity"  window. Each  "Decode" message  sent will  have the
// "New" flag set to false so that they can be distinguished from
// new decodes. After  all the old decodes have  been broadcast \
// "Status" message  is also broadcast.  If the server  wishes to
// determine  the  status  of  a newly  discovered  client;  this
// message should be used.

class WSJTX_Replay : public WSJTX_Packet {
public:
    WSJTX_Replay(const std::vector<uint8_t>& pkt, int idx) : WSJTX_Packet(pkt, idx) {
        // Add specific initialization code here for WSJTX_Replay
    }
};


// Packet Type 8 Halt Tx (IN to client This is a message to be sent to the client)
//
// The server may stop a client from transmitting messages either
// immediately or at  the end of the  current transmission period
// using this message.

class WSJTX_HaltTx : public WSJTX_Packet {
public:
    WSJTX_HaltTx(const std::vector<uint8_t>& pkt, int idx) : WSJTX_Packet(pkt, idx) {
        // Additional initialization specific to WSJTX_HaltTx
    }
};


// Packet Type 9 (IN to client This is a message to be sent to the client)
//
// This message  allows the server  to set the current  free text
// message content. Sending this  message with a non-empty "Text"
// field is equivalent to typing  a new message (old contents are
// discarded) in to  the WSJT-X free text message  field or "Tx5"
// field (both  are updated) and if  the "Send" flag is  set then
// clicking the "Now" radio button for the "Tx5" field if tab one
// is current or clicking the "Free  msg" radio button if tab two
// is current.
//
// It is the responsibility of the  sender to limit the length of
// the  message   text  and   to  limit   it  to   legal  message
// characters. Despite this,  it may be difficult  for the sender
// to determine the maximum message length without reimplementing
// the complete message encoding protocol. Because of this is may
// be better  to allow any  reasonable message length and  to let
// the WSJT-X application encode and possibly truncate the actual
// on-air message.

// If the  message text is  empty the  meaning of the  message is
// refined  to send  the  current free  text  unchanged when  the
// "Send" flag is set or to  clear the current free text when the
// "Send" flag is  unset.  Note that this API does  not include a
// command to  determine the  contents of  the current  free text
// message.

class WSJTX_FreeText : public WSJTX_Packet {
public:
    WSJTX_FreeText(const std::vector<uint8_t>& pkt, int idx) : WSJTX_Packet(pkt, idx) {
        // Additional initialization specific to WSJTX_FreeText
    }
};


// Packet Type 10 WSPR Decode

// The decode message is sent when  a new decode is completed, in
// this case the 'New' field is true. It is also used in response
// to  a "Replay"  message where  each  old decode  in the  "Band
// activity" window, that  has not been erased, is  sent in order
// as  a one  of  these  messages with  the  'New'  field set  to
// false.  See   the  "Replay"  message  below   for  details  of
// usage. The off air field indicates that the decode was decoded
// from a played back recording.

class WSJTX_WSPRDecode : public WSJTX_Packet {
public:
    WSJTX_WSPRDecode(const std::vector<uint8_t>& pkt, int idx) : WSJTX_Packet(pkt, idx) {
        // Additional initialization specific to WSJTX_WSPRDecode
    }
};



