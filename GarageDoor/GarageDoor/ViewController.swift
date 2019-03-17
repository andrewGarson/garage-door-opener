//
//  ViewController.swift
//  GarageDoor
//
//  Created by Andrew Garson on 3/17/19.
//  Copyright © 2019 Andrew Garson. All rights reserved.
//

import UIKit
import CocoaMQTT

let presence = "presence"
let commands = "commands"

class ViewController: UIViewController {
  
  let clientID = "CocoaMQTT-" + String(ProcessInfo().processIdentifier)
  private var mqtt: CocoaMQTT?
  
  @IBOutlet weak var textView: UITextView!
  override func viewDidLoad() {
    super.viewDidLoad()
    textView.isEditable = false
    
    let mqtt = CocoaMQTT(clientID: clientID, host: "192.168.1.22", port: 1883)
    mqtt.keepAlive = 60
    mqtt.delegate = self
    mqtt.connect()
    self.mqtt = mqtt
  }
  
  func append(_ line: String) {
    textView.text.append("\(line)\n")
  }

  @IBAction func force(_ sender: Any) {
    append("Trying to force")
    mqtt?.publish(commands, withString: "force\0")
  }
  
  @IBAction func open(_ sender: Any) {
    append("Trying to open")
    mqtt?.publish(commands, withString: "open\0")
  }
  
  @IBAction func close(_ sender: Any) {
    append("Trying to close")
    mqtt?.publish(commands, withString: "close\0")
  }
}


extension ViewController : CocoaMQTTDelegate {
  func mqtt(_ mqtt: CocoaMQTT, didConnectAck ack: CocoaMQTTConnAck) {
    append("Connected")
    mqtt.subscribe(presence)
  }
  
  func mqtt(_ mqtt: CocoaMQTT, didReceiveMessage message: CocoaMQTTMessage, id: UInt16) {
    append("Received Message - \(message.topic):")
    append("\t\(message.string ?? "")")
  }
  
  func mqtt(_ mqtt: CocoaMQTT, didSubscribeTopic topic: String) {
    if topic == presence {
      mqtt.publish(presence, withString: clientID)
    }
    append("Subscribed to \(topic)")
  }
  
  func mqttDidDisconnect(_ mqtt: CocoaMQTT, withError err: Error?) {
    append("Disconnected")
  }
  
  func mqtt(_ mqtt: CocoaMQTT, didPublishMessage message: CocoaMQTTMessage, id: UInt16) { }
  
  func mqtt(_ mqtt: CocoaMQTT, didPublishAck id: UInt16) { }

  func mqtt(_ mqtt: CocoaMQTT, didUnsubscribeTopic topic: String) {}
  
  func mqttDidPing(_ mqtt: CocoaMQTT) { }
  
  func mqttDidReceivePong(_ mqtt: CocoaMQTT) { }
}

