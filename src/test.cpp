#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::depends(BH)]]

//
// examples/fetch_cxx03.cpp
// ------------------------
//
// Copyright (c) 2015 Daniel Joos
//
// Distributed under MIT license. (See file LICENSE)
//
// ----------------------------------
//
// This example shows how to create a 'FetchRequest' to get messages for a
// specific Topic & partition. On success, all received messages will be print
// to stdout.
//

#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <libkafka_asio/libkafka_asio.h>

using libkafka_asio::Connection;
using libkafka_asio::FetchRequest;
using libkafka_asio::FetchResponse;
using libkafka_asio::MessageAndOffset;

std::string BytesToString(const libkafka_asio::Bytes& bytes)
{
  if (!bytes || bytes->empty())
  {
    return "";
  }
  return std::string((const char*) &(*bytes)[0], bytes->size());
};

void PrintMessage(const MessageAndOffset& message)
{
  Rcpp::Rcout << BytesToString(message.value()) << std::endl;
}

void HandleFetch(const Connection::ErrorCodeType& err,
                 const FetchResponse::OptionalType& response)
{
  if (err)
  {
    std::cerr
    << "Error: " << boost::system::system_error(err).what()
    << std::endl;
    return;
  }
  std::for_each(response->begin(), response->end(), &PrintMessage);
}

//' @export
//' @useDynLib Kafkar
// [[Rcpp::export()]]
int test(std::string address)
{
  Connection::Configuration configuration;
  configuration.auto_connect = true;
  configuration.client_id = "libkafka_asio_example";
  configuration.socket_timeout = 10000;
  configuration.SetBrokerFromString(address);
  
  boost::asio::io_service ios;
  Connection connection(ios, configuration);
  
  // Create a 'Fetch' request and try to get data for partition 0 of topic
  // 'mytopic', starting with offset 1
  FetchRequest request;
  request.FetchTopic("mytopic", 0, 1);
  
  // Send the prepared fetch request.
  // The connection will attempt to automatically connect to the broker,
  // specified in the configuration.
  connection.AsyncRequest(request, &HandleFetch);
  
  // Let's go!
  ios.run();
  return 0;
}