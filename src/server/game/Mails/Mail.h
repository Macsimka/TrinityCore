/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRINITY_MAIL_H
#define TRINITY_MAIL_H

#include "Common.h"
#include "DatabaseEnvFwd.h"
#include "ObjectGuid.h"
#include <map>

struct CalendarEvent;
class AuctionHouseObject;
class BlackMarketEntry;
class Item;
class Object;
class Player;

#define MAIL_BODY_ITEM_TEMPLATE 8383                        // - plain letter, A Dusty Unsent Letter: 889
#define MAX_CLIENT_MAIL_ITEMS 12                            // max number of items a player is allowed to attach
#define MAX_MAIL_ITEMS 16

enum MailMessageType
{
    MAIL_NORMAL                 = 0,
    MAIL_AUCTION                = 2,
    MAIL_CREATURE               = 3,                        // client send CMSG_CREATURE_QUERY on this mailmessagetype
    MAIL_GAMEOBJECT             = 4,                        // client send CMSG_GAMEOBJECT_QUERY on this mailmessagetype
    MAIL_CALENDAR               = 5,
    MAIL_BLACKMARKET            = 6,
    MAIL_COMMERCE_AUCTION       = 7,                        // wow token auction
    MAIL_AUCTION_2              = 8,
    MAIL_ARTISANS_CONSORTIUM    = 9                         // crafting orders
};

enum MailCheckMask
{
    MAIL_CHECK_MASK_NONE        = 0x00,
    MAIL_CHECK_MASK_READ        = 0x01,
    MAIL_CHECK_MASK_RETURNED    = 0x02,                     /// This mail was returned. Do not allow returning mail back again.
    MAIL_CHECK_MASK_COPIED      = 0x04,                     /// This mail was copied. Do not allow making a copy of items in mail.
    MAIL_CHECK_MASK_COD_PAYMENT = 0x08,
    MAIL_CHECK_MASK_HAS_BODY    = 0x10                      /// This mail has body text.
};

// gathered from Stationery.dbc
enum MailStationery
{
    MAIL_STATIONERY_TEST    = 1,
    MAIL_STATIONERY_DEFAULT = 41,
    MAIL_STATIONERY_GM      = 61,
    MAIL_STATIONERY_AUCTION = 62,
    MAIL_STATIONERY_VAL     = 64,                           // Valentine
    MAIL_STATIONERY_CHR     = 65,                           // Christmas
    MAIL_STATIONERY_ORP     = 67                            // Orphan
};

enum MailState
{
    MAIL_STATE_UNCHANGED = 1,
    MAIL_STATE_CHANGED   = 2,
    MAIL_STATE_DELETED   = 3
};

enum MailShowFlags
{
    MAIL_SHOW_UNK0    = 0x0001,
    MAIL_SHOW_DELETE  = 0x0002,                             // forced show delete button instead return button
    MAIL_SHOW_AUCTION = 0x0004,                             // from old comment
    MAIL_SHOW_UNK2    = 0x0008,                             // unknown, COD will be shown even without that flag
    MAIL_SHOW_RETURN  = 0x0010
};

class TC_GAME_API MailSender
{
    public:                                                 // Constructors
        MailSender(MailMessageType messageType, ObjectGuid::LowType sender_guidlow_or_entry, MailStationery stationery = MAIL_STATIONERY_DEFAULT)
            : m_messageType(messageType), m_senderId(sender_guidlow_or_entry), m_stationery(stationery)
        {
        }
        MailSender(Object* sender, MailStationery stationery = MAIL_STATIONERY_DEFAULT);
        MailSender(CalendarEvent* sender);
        MailSender(AuctionHouseObject const* sender);
        MailSender(BlackMarketEntry* sender);
        MailSender(Player* sender);
        MailSender(uint32 senderEntry);
    public:                                                 // Accessors
        MailMessageType GetMailMessageType() const { return m_messageType; }
        ObjectGuid::LowType GetSenderId() const { return m_senderId; }
        MailStationery GetStationery() const { return m_stationery; }
    private:
        MailMessageType m_messageType;
        ObjectGuid::LowType m_senderId;                                  // player low guid or other object entry
        MailStationery m_stationery;
};

class TC_GAME_API MailReceiver
{
    public:                                                 // Constructors
        explicit MailReceiver(ObjectGuid::LowType receiver_lowguid) : m_receiver(nullptr), m_receiver_lowguid(receiver_lowguid) { }
        MailReceiver(Player* receiver);
        MailReceiver(Player* receiver, ObjectGuid::LowType receiver_lowguid);
        MailReceiver(Player* receiver, ObjectGuid receiverGuid);
    public:                                                 // Accessors
        Player* GetPlayer() const { return m_receiver; }
        ObjectGuid::LowType GetPlayerGUIDLow() const { return m_receiver_lowguid; }
    private:
        Player* m_receiver;
        ObjectGuid::LowType m_receiver_lowguid;
};

class TC_GAME_API MailDraft
{
    typedef std::map<ObjectGuid::LowType, Item*> MailItemMap;

    public:                                                 // Constructors
        explicit MailDraft(uint16 mailTemplateId, bool need_items = true)
            : m_mailTemplateId(mailTemplateId), m_mailTemplateItemsNeed(need_items), m_money(0), m_COD(0)
        { }
        MailDraft(std::string const& subject, std::string const& body)
            : m_mailTemplateId(0), m_mailTemplateItemsNeed(false), m_subject(subject), m_body(body), m_money(0), m_COD(0) { }
    public:                                                 // Accessors
        uint16 GetMailTemplateId() const { return m_mailTemplateId; }
        std::string const& GetSubject() const { return m_subject; }
        uint64 GetMoney() const { return m_money; }
        uint64 GetCOD() const { return m_COD; }
        std::string const& GetBody() const { return m_body; }

    public:                                                 // modifiers
        MailDraft& AddItem(Item* item);
        MailDraft& AddMoney(uint64 money) { m_money = money; return *this; }
        MailDraft& AddCOD(uint64 COD) { m_COD = COD; return *this; }

    public:                                                 // finishers
        void SendReturnToSender(uint32 sender_acc, ObjectGuid::LowType sender_guid, ObjectGuid::LowType receiver_guid, CharacterDatabaseTransaction trans);
        void SendMailTo(CharacterDatabaseTransaction trans, MailReceiver const& receiver, MailSender const& sender, MailCheckMask checked = MAIL_CHECK_MASK_NONE, uint32 deliver_delay = 0);

    private:
        void deleteIncludedItems(CharacterDatabaseTransaction trans, bool inDB = false);
        void prepareItems(Player* receiver, CharacterDatabaseTransaction trans);                // called from SendMailTo for generate mailTemplateBase items

        uint16      m_mailTemplateId;
        bool        m_mailTemplateItemsNeed;
        std::string m_subject;
        std::string m_body;

        MailItemMap m_items;                                // Keep the items in a map to avoid duplicate guids (which can happen), store only low part of guid

        uint64 m_money;
        uint64 m_COD;
};

struct MailItemInfo
{
    ObjectGuid::LowType item_guid;
    uint32 item_template;
};
typedef std::vector<MailItemInfo> MailItemInfoVec;

struct Mail
{
    uint64 messageID;
    uint8 messageType;
    uint8 stationery;
    uint16 mailTemplateId;
    ObjectGuid::LowType sender;  // TODO: change to uint64 and store full guids
    ObjectGuid::LowType receiver;
    std::string subject;
    std::string body;
    std::vector<MailItemInfo> items;
    std::vector<ObjectGuid::LowType> removedItems;
    time_t expire_time;
    time_t deliver_time;
    uint64 money;
    uint64 COD;
    uint32 checked;
    MailState state;

    void AddItem(ObjectGuid::LowType itemGuidLow, uint32 item_template)
    {
        MailItemInfo mii;
        mii.item_guid = itemGuidLow;
        mii.item_template = item_template;
        items.push_back(mii);
    }

    bool RemoveItem(ObjectGuid::LowType item_guid)
    {
        for (MailItemInfoVec::iterator itr = items.begin(); itr != items.end(); ++itr)
        {
            if (itr->item_guid == item_guid)
            {
                items.erase(itr);
                return true;
            }
        }
        return false;
    }

    bool HasItems() const { return !items.empty(); }
};

#endif
