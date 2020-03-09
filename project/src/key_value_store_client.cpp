#include <thread>

#include "key_value_store_client.h"

using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;

KeyValueStoreClient::KeyValueStoreClient(std::shared_ptr<Channel> channel)
    : stub_(KeyValueStore::NewStub(channel)) {}

void KeyValueStoreClient::put(const std::string& key,
                              const std::string& value) {
  // Data we are sending to the server.
  std::cout << " reached KeyValueStoreClient's put call successfully ";

  PutRequest request;
  request.set_key(key);
  request.set_value(value);

  // Container for the data we expect from the server.
  PutReply reply;

  // Context for the client. It could be used to convey extra information to
  // the server and/or tweak certain RPC behaviors.
  ClientContext context;

  // The actual RPC.
  Status status = stub_->put(&context, request, &reply);

  // Act upon its status.
  if (status.ok()) {
    std::cout
        << "PutReply was returned to Key Value store client with Status::OK"
        << std::endl;
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
  }
}

std::vector<GetReply> KeyValueStoreClient::get(const std::string& key) {
  std::cout << " reached KeyValueStoreClient's get call successfully ";
  std::cout << " with key: " << key << std::endl;

  ClientContext context;

  std::shared_ptr<ClientReaderWriter<GetRequest, GetReply> > stream(
      stub_->get(&context));

  std::thread writer([stream, &key]() {
    // Create the request and write it into the stream
    GetRequest only_request;
    only_request.set_key(key);
    std::vector<GetRequest> requests{only_request};
    for (const GetRequest& req : requests) {
      std::cout << "Sending message " << req.key() << std::endl;
      stream->Write(req);
    }
    stream->WritesDone();
  });

  std::vector<GetReply> replies;
  GetReply reply;
  while (stream->Read(&reply)) {
    std::cout << "Got message " << reply.value() << std::endl;
    replies.push_back(reply);
  }

  writer.join();
  Status status = stream->Finish();
  if (!status.ok()) {
    std::cout << "kv_store_client get rpc failed." << std::endl;
  }
  return replies;
}

void KeyValueStoreClient::remove(const std::string& key) {
  RemoveRequest request;
  request.set_key(key);

  // Container for the data we expect from the server.
  RemoveReply reply;

  // Context for the client. It could be used to convey extra information to
  // the server and/or tweak certain RPC behaviors.
  ClientContext context;

  // The actual RPC.
  Status status = stub_->remove(&context, request, &reply);

  // Act upon its status.
  if (status.ok()) {
    std::cout
        << "RemoveReply was returned to Key Value store client with Status::OK"
        << std::endl;
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
  }
}