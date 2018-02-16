
var net = require("net");

var HOST = '192.168.1.200';
var PORT = '10470';
var Service, Characteristic, DoorState;;
var openflag = '6';
var curflag = 'busy';
module.exports = (homebridge) => {
  Service = homebridge.hap.Service;
  Characteristic = homebridge.hap.Characteristic;
  DoorState = homebridge.hap.Characteristic.CurrentDoorState
  homebridge.registerAccessory('homebridge-garage2arduino', 'garage2arduino', Garage2Ardiuno);
};

//constructor function
function Garage2Ardiuno(log, config){

    //get config values for gobal config value;
    this.name = config['name'];
    PORT = config['serverPort'] || 10470;
	HOST = config['host'] || 10470;
    //this.simulateTimeOpening = config['simulateTimeOpening'] || 15;// i used to simulate garage open and close time.
    //this.simulateTimeClosing = config['simulateTimeClosing'] || 15;
    //initial setup

    this.log = log;
    this.service = new Service.GarageDoorOpener(this.name, this.name);
    this.garage2ArdiunoWorkOnInit(this.service);

    this.informationService = new Service.AccessoryInformation();
    this.informationService
      .setCharacteristic(Characteristic.Manufacturer, 'Garage 2 Ardiuno')
      .setCharacteristic(Characteristic.Model, 'a garage door controler')
      .setCharacteristic(Characteristic.SerialNumber, 'version 1.00');
}

//prototype definetion
Garage2Ardiuno.prototype = {


  //function get service API 
  getServices: function (){
    return [this.informationService, this.service];
  },

  //function workonInit
  garage2ArdiunoWorkOnInit: function (service) {
    this.service.setCharacteristic(Characteristic.TargetDoorState, Characteristic.TargetDoorState.CLOSED);
    this.service.setCharacteristic(Characteristic.CurrentDoorState, Characteristic.CurrentDoorState.CLOSED);
    service.getCharacteristic(Characteristic.TargetDoorState).on('get', this.G2AGet.bind(this));
    service.getCharacteristic(Characteristic.TargetDoorState).on('set', this.G2ASet.bind(this));
  },

  //function get
  G2AGet: function(callback){
    console.log("call get");
    //var curTimePoint = new Date().getTime()/1000;

    var curDoorStat = this.service.getCharacteristic(Characteristic.CurrentDoorState).value;

    if(curDoorStat === DoorState.OPENING)
    {

      //var openTimeGap = curTimePoint-this.openTimePoint;
      //if(openTimeGap > this.simulateTimeOpening)

      if(curflag == 0)
      {
        this.service.setCharacteristic(Characteristic.CurrentDoorState,Characteristic.CurrentDoorState.OPEN);
        console.log("Open ");
        callback(null,DoorState.OPEN);
      }
      else
      {
        this.service.setCharacteristic(Characteristic.CurrentDoorState,Characteristic.CurrentDoorState.OPENING);
        console.log("Opeing ");
        callback(null,DoorState.OPEN);
      }

    }
    else if(curDoorStat === DoorState.CLOSING)
    {
      //var closeTimeGap = curTimePoint-this.closeTimePoint;
      //if(closeTimeGap > this.simulateTimeClosing)

      if(curflag == 0)
      {
        this.service.setCharacteristic(Characteristic.CurrentDoorState,Characteristic.CurrentDoorState.CLOSED);
        console.log("Closed ");
        callback(null,DoorState.CLOSED);
      }
      else
      {
        this.service.setCharacteristic(Characteristic.CurrentDoorState, Characteristic.CurrentDoorState.CLOSING);
        console.log("Closing ");
        callback(null,DoorState.CLOSED);
      }

    }
    else
    {
      console.log("status：cur: "+curDoorStat+"open"+DoorState.OPEN+"closed" + DoorState.CLOSED+"opening" + DoorState.OPENING+"closing"+DoorState.CLOSING);
      callback(null,curDoorStat);
    }

  },


  //function set
  G2ASet: function(value,callback)
  {

    console.log("call set value",value,Characteristic.TargetDoorState.OPEN,Characteristic.TargetDoorState.CLOSED);
    if (value === Characteristic.TargetDoorState.OPEN)
    {
      console.log("GarageDoor Open");
      //this.openTimePoint = new Date().getTime()/1000;
      openflag = 2;//2 mean open
      curflag = 1;//1 mean busy 0 mean idel
      callback();
      this.service.setCharacteristic(Characteristic.CurrentDoorState,Characteristic.CurrentDoorState.OPENING);
    }

    else if(value === Characteristic.TargetDoorState.CLOSED)
    {
      console.log("GarageDoor Close");
      //this.closeTimePoint = new Date().getTime()/1000;
      openflag = 5;//mean close 6 mean idel
      curflag = 1;
      callback();
      this.service.setCharacteristic(Characteristic.CurrentDoorState, Characteristic.CurrentDoorState.CLOSING);
    }

  }

}

net.createServer(function(sock)
{
  console.log("connected:" + sock.remoteAddress + ":" + sock.remotePort);
  sock.on('data', function(data) {
		
        console.log('RECV: ' + sock.remoteAddress + ': ' + data);

        if(openflag != 6)
        {
          curflag = 1;
        }
        else if(data.indexOf("STATUS:6") == -1) 
        {

          curflag = 1;
        }
        else
        {
          curflag = 0;
        }

        setTimeout(function(){sock.write("STATUS:"+openflag+"\n");  
        console.log("STATUS:"+openflag +"," + curflag);
        openflag = 6;

      },200);
  });
  sock.on('error', function(err) {
           console.log("error catch-",err);
  });
}).listen(PORT, HOST);
