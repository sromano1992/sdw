package com.sdw.main;

public class Settings {
	private int backlightValue, id;
	private String name;
	
	public Settings(int backlightValue, String name) {
		super();
		this.backlightValue = backlightValue;
		this.name = name;
	}
	public Settings() {
	}
	
	public int getBacklightValue() {
		return backlightValue;
	}
	public void setBacklightValue(int backlightValue) {
		this.backlightValue = backlightValue;
	}
	public String getName() {
		return name;
	}
	public void setName(String name) {
		this.name = name;
	}
	public void setId(int parseInt) {
		this.id = id;
	}
	public int getId() {
		return this.id = id;
	}	
}
