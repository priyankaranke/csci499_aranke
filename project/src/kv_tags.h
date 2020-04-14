#ifndef KV_TAGS_H
#define KV_TAGS_H

namespace kv_tags {
// Identifiers that specify which string is prepended to the key before adding
// to key value store
const std::string kUserFollowing = "user_following:";
const std::string kUserFollower = "user_follower:";
const std::string kWarblePost = "warble_post:";
const std::string kWarbleChildren = "warble_children:";

// prefix used to read and write latest warble from kvstore
const std::string kLatestWarbleString = "latest_warble_id:";
}  // namespace kv_tags

#endif