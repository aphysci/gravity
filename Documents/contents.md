# Table of Contents        {#contents}
\if cpp
## Gravity CPP API 

<ul style="list-style-type: none;"> 
  <li> Getting started with Gravity and connecting to the Service Directory. </li>
  <ul>
  <li> Initialize a \ref gravity::GravityNode "GravityNode" with a unique componentID through \ref gravity::GravityNode::init() "GravityNode::init()" to connect to the Service Directory.</li>
  <li> The Service Directory is what manages all of the \ref gravity::GravityNode "GravityNodes", data subscriptions, publications, requests, and responses on the Gravity Network. </li>
  </ul>

  <li> Data flows through the GravityNetwork as \ref gravity::GravityDataProduct "GravityDataProducts". </li>
  <ul> 
  <li> \ref gravity::GravityDataProduct "GravityDataProducts" are easily interchangeable with Google Protocol Buffer messages. </li>
  <ul> 
  <li> Use \ref gravity::GravityDataProduct::setData() "GravityDataProduct::setData()" to move Google Protocol Buffer messages into the class. </li>
  <li> Use \ref gravity::GravityDataProduct::populateMessage "GravityDataProduct::populateMessage()" to transfer the data from the class into a Google Protocol Buffer message. </li>
  </ul>
  <li> \ref gravity::GravityDataProduct "GravityDataProducts" are sent through the pub-sub method. </li> 
  <ul> 
  <li> Before publishing, register the ID of each data product you wish to publish with \ref gravity::GravityNode::registerDataProduct() "GravityNode::registerDataProduct()". </li>
  <li> Then publish using \ref gravity::GravityNode::publish() "GravityNode::publish()". </li>
  <li> Register a \ref gravity::GravitySubscriber "GravitySubscriber" through \ref gravity::GravityNode::subscribe() "GravityNode::subscribe()" to recieve subscriptions. </li>
  <li> Register a \ref gravity::GravitySubscriptionMonitor "GravitySubscriptionMonitor" through \ref gravity::GravityNode::setSubscriptionTimeoutMonitor() "GravityNode::setSubscriptionTimeoutMonitor()" to receive subscription timeout alerts. </li>
  </ul>
  </ul>
  
  <li> Requests/responses can be made through the GravityNetwork. Just like data, these messages travel as \ref gravity::GravityDataProduct "GravityDataProducts". </li>
  <ul> 
  <li> Make an asynchronous request and register a \ref gravity::GravityRequestor "GravityRequestor" to receive the response through \ref gravity::GravityNode "GravityNode::request()". </li>
  <li> Register a \ref gravity::GravityServiceProvider "GravityServiceProvider" through \ref gravity::GravityNode::registerService() "GravityNode::registerService()" to receive the request and send a response. </li>
  </ul>

  <li> Heartbeats monitor the "alive" status of published data products on \ref gravity::GravityNode "GravityNodes". </li>
  <ul> 
  <li> Use \ref gravity::GravityNode::startHeartbeat() "GravityNode::startHeartbeat()" to start a heartbeat on a \ref gravity::GravityNode "GravityNode". </li>
  <li> Register a \ref gravity::GravityHeartbeatListener "GravityHeartbeatListener" through \ref gravity::GravityNode::registerHeartbeatListener() "GravityNode::registerHeartbeatListener()" to receive these heartbeat updates. </li>
  </ul>
 
  <li>In summary, the following interfaces be inherited to interact with the Gravity Network: </li>
  <ul> 
    <li>\ref gravity::GravitySubscriber "GravitySubscriber" to receive data subscriptions. 
    <li>\ref gravity::GravitySubscriptionMonitor "GravitySubscriptionMonitor" to receive data timeout alerts. 
    <li>\ref gravity::GravityRequestor "GravityRequestor" to receive a response from an asynchronous request. </li>
    <li>\ref gravity::GravityServiceProvider "GravityServiceProvider" to receive a request and send a response. </li>
    <li>\ref gravity::GravityHeartbeatListener "GravityHeartbeatListener" to receive information about data connection losses. </li>
  </ul>

  <li> Gravity also contains a framework to log messages. </li>
  <ul> 
  <li> Use the static \ref gravity::Log "Log" class to create a log messages. </li>
  <li> Although there are methods to create \ref gravity::Logger "Loggers", at initialization of the GravityNode a console \ref gravity::Logger "Logger" and a file \ref gravity::Logger "Logger" are created. </li>
  <li> By default, the only logged messages are of \ref gravity::Log::LogLevel "LogLevel::WARNING" and above. This can be overridden in the Gravity.ini file. See the Wiki for examples. </li>
  </ul>

  <li> TBD \ref gravity::FutureResponse "FutureResponse" </li> 

</ul>
\endif

\if java
## Gravity java API 

<ul style="list-style-type: none;"> 
  <li>Java table of contents</li>
</ul>
\endif

