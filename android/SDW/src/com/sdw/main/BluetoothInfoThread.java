package com.sdw.main;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Calendar;


import java.util.List;
import java.util.Locale;

import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.location.Address;
import android.location.Geocoder;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.TextView;

public class BluetoothInfoThread extends Thread {
	private static final long LOOP_OFF_TIME = 500;
	private int INTERVAL = 30000;
	private ConnectedThread mConnectedThread;
	private String city, name = "---", operatorName = "---";
	private int backlight = 255;
	private TextView textViewName;
	private static boolean LOOP_ON;
	public static boolean IN_LOOP;
	private TelephonyManager telephonyManager;
	private MySQLiteHelper db;
	private Settings mySetting;
	private static ArrayList<Message> myMessage;
	private static boolean UPDATE_NOTIFICATION;
	private Context mainContext, baseContext;
	private MainActivity mainActivity;
	
	public BluetoothInfoThread(ConnectedThread mConnectedThread, TextView textViewName, TelephonyManager telephonyMgr, Context context, Context baseContext, MainActivity mainActivity) {
		super();
		myMessage = new ArrayList<Message>();
		this.mConnectedThread = mConnectedThread;
		this.textViewName = textViewName;
		telephonyManager = telephonyMgr;
		LOOP_ON = true;
		mainContext = context;
		this.baseContext = baseContext;
		this.mainActivity = mainActivity;
	}

	public void run(){
		while(true){
			if(LOOP_ON){
				IN_LOOP = true;
				Calendar c = Calendar.getInstance(); 
				int hours = c.get(Calendar.HOUR_OF_DAY);
				int minutes = c.get(Calendar.MINUTE);
				String minAsString = minutes + "";
				if(minutes < 10)
					minAsString = "0" + minAsString;			
				//int seconds = c.get(Calendar.SECOND);
				int day = c.get(Calendar.DAY_OF_MONTH);
				int month = c.get(Calendar.MONTH)+1;
				int year = c.get(Calendar.YEAR);
				int dayOfWeek = c.get(Calendar.DAY_OF_WEEK);
				//checkGps//
				Handler handler = new Handler(Looper.getMainLooper()); 
				handler.postDelayed(new Runnable() {
				  public void run() {
					checkGps(false);
				  }
				}, 1000 );
				//**********//
				String cityToPrint = city;
				if(cityToPrint == null)
					cityToPrint = "NO GPS";
				if(!textViewName.getText().equals(""))
					name = textViewName.getText().toString();
				if(mySetting != null){
					if(!name.equals(mySetting.getName())){
						mySetting.setName(name);
						//mySetting.setBacklightValue(backlight);
						db.updateSettings(mySetting);
					}
				}
				else{
					mySetting = new Settings(backlight, name);
					db.addSettingsRow(name, backlight);
				}
				String dayOfWeek_String = getDayOfWeekAsString(dayOfWeek);
				operatorName = telephonyManager.getNetworkOperatorName();
				try{
					if(mConnectedThread != null){
						mConnectedThread.write("P;" + hours + ":" + minAsString + ";" + day + "/" + month + "/"
							+ year + ";" + dayOfWeek_String + ";" + cityToPrint + ";" + name + ";" + backlight + ";" + operatorName + "_");
						if(UPDATE_NOTIFICATION){
							try {
								IN_LOOP = false;
								Thread.sleep(30000);
							} catch (InterruptedException e) {
								e.printStackTrace();
							}
							IN_LOOP = true;
							if(myMessage.size() == 2){
								mConnectedThread.write("A;" + myMessage.get(0).toString()
													  + ";" + myMessage.get(1).toString() + "_");
							}
							UPDATE_NOTIFICATION = false;
						}
					}
				}catch(Exception e){
					e.printStackTrace();
					reconnecting();
				}
				try {
					IN_LOOP = false;
					Thread.sleep(INTERVAL);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}
	}

	private void reconnecting() {
		while(mainActivity.connectToDevice() == false){
			Log.i("CONNECTING", "trying to connect...");
		}
	}

	private String getDayOfWeekAsString(int dayOfWeek) {
		if(dayOfWeek == 2)
			return "MONDAY";
		if(dayOfWeek == 3)
			return "TUESDAY";
		if(dayOfWeek == 4)
			return "WEDNESDAY";
		if(dayOfWeek == 5)
			return "THURSDAY";
		if(dayOfWeek == 6)
			return "FRIDAY";
		if(dayOfWeek == 7)
			return "SATURDAY";
		if(dayOfWeek == 1)
			return "SUNDAY";
		return "no day";
	}

	public String getCity() {
		return city;
	}

	public void setCity(String city) {
		this.city = city;
	}

	public String getOwnerName() {
		return name;
	}

	public void setOwnerName(String name) {
		this.name = name;
	}

	public int getBacklight() {
		return backlight;
	}

	public void setBacklight(int backlight) {
		this.backlight = backlight;
		if(mySetting != null){
			mySetting.setBacklightValue(backlight);
			db.updateSettings(mySetting);
		}
	}
	
	public void turnOnFlash(){
		while(IN_LOOP){}
		try {
			mConnectedThread.write("on");
		} catch (Exception e) {
			e.printStackTrace();
			reconnecting();
		}
	}
	
	public void turnOffFlash(){
		while(IN_LOOP){}
		try {
			mConnectedThread.write("off");
		} catch (Exception e) {
			e.printStackTrace();
			reconnecting();
		}
	}

	public void printHome() {
		while(IN_LOOP){}
		try {
			mConnectedThread.write("HOME");
		} catch (Exception e) {
			e.printStackTrace();
			reconnecting();
		}
	}
	
	/*public void printCallInfo(String name, String number){
		LOOP_ON = false;
		try {
			Thread.sleep(LOOP_OFF_TIME);
			mConnectedThread.write("C;" + name + ";" + number + "_");
			Thread.sleep(LOOP_OFF_TIME);
			LOOP_ON = true;
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}*/

	public void set_mConnectedThread(ConnectedThread mConnectedThread) {
		this.mConnectedThread = mConnectedThread;
	}

	public boolean isLOOP_ON() {
		return LOOP_ON;
	}

	public void setLOOP_ON(boolean lOOP_ON) {
		LOOP_ON = lOOP_ON;
	}

	public static long getLoopOffTime() {
		return LOOP_OFF_TIME;
	}

	public static ArrayList<Message> getMyMessage() {
		return myMessage;
	}

	public static void setMyMessage(ArrayList<Message> myMessage) {
		BluetoothInfoThread.myMessage = myMessage;
		UPDATE_NOTIFICATION = true;
	}

	public void setDbHelper(MySQLiteHelper db) {
		this.db = db;
	}

	public void setSettings(Settings mySetting) {
		this.mySetting = mySetting;
	}

	public void checkGps(boolean reqIfDisabled) {
		//location
		LocationManager locationManager = (LocationManager)	mainContext.getSystemService(Context.LOCATION_SERVICE);
		LocationListener locationListener = new MyLocationListener();
		locationManager.requestLocationUpdates(LocationManager.GPS_PROVIDER, 5000, 10, locationListener);
		boolean network_enabled = locationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER);	
		if(network_enabled){			
			getLocation(locationManager);
		}
	}

	

	public void getLocation(LocationManager locationManager) {
		Location location = locationManager.getLastKnownLocation(LocationManager.NETWORK_PROVIDER);
		Geocoder gcd = new Geocoder(baseContext, Locale.getDefault());
		List<Address> addresses;
		try {
		    addresses = gcd.getFromLocation(location.getLatitude(),
		    		location.getLongitude(), 1);
		    if (addresses.size() > 0)
		        System.out.println(addresses.get(0).getLocality());
		    this.setCity(addresses.get(0).getLocality());
		}
		catch(Exception e){
			e.printStackTrace();
		}
	}
	private class MyLocationListener implements LocationListener {

        private String TAG = "SDW_infothread";

		@Override
        public void onLocationChanged(Location loc) {
            /*Toast.makeText(
                    getBaseContext(),
                    "Location changed: Lat: " + loc.getLatitude() + " Lng: "
                        + loc.getLongitude(), Toast.LENGTH_SHORT).show();*/
            String longitude = "Longitude: " + loc.getLongitude();
            //Log.v(TAG , longitude);
            String latitude = "Latitude: " + loc.getLatitude();
            //Log.v(TAG, latitude);

            /*------- To get city name from coordinates -------- */
            String cityName = null;
            Geocoder gcd = new Geocoder(baseContext, Locale.getDefault());
            List<Address> addresses;
            try {
                addresses = gcd.getFromLocation(loc.getLatitude(),
                        loc.getLongitude(), 1);
                if (addresses.size() > 0)
                    System.out.println(addresses.get(0).getLocality());
                cityName = addresses.get(0).getLocality();
            }
            catch (IOException e) {
                e.printStackTrace();
            }
            String s = longitude + "\n" + latitude + "\n\nMy Current City is: "
                + cityName;
            //Log.i("GPS", s);
            setCity(cityName);
        }

        @Override
        public void onProviderDisabled(String provider) {}

        @Override
        public void onProviderEnabled(String provider) {}

        @Override
        public void onStatusChanged(String provider, int status, Bundle extras) {}
    }
}
