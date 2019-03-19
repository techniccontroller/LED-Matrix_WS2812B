package com.techniccontroller.bluetoothremoteledmatrix;

/**
 * Created by Edgar on 18.03.2016.
 */
public class Bluetoothgeraet {

    private String name;
    private String adresse;

    public Bluetoothgeraet(String name, String adresse) {
        this.name = name;
        this.adresse = adresse;
    }

    public String getAdresse() {
        return adresse;
    }
}
