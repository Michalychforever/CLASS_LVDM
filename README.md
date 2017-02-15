# CLASS LVDM
This is the modification of the CLASS code v1.7 that incorporates the cosmological model of Lorentz invariance violation (LV) in gravity and dark matter, presented in arXiv:1209.0464. This code was used in the paper arXiv:1410.6514.
Compared to the usual CLASS code, it containts 4 new parameters (see arXiv:1410.6514 for more detail):

* alpha, beta, lambda - charachterize LV in the gravity sector. Note that in order to restore LCDM, one has to put these parameters very small, say 1.e-7, as putting these parameters exactly to zero produces singularities
* Y - charachterizes LV in the dark matter sector. Put to 0 if you want Lorentz invariant dark matter

In order to use the code, download the .zip file and add the file bessels.dat into the folder
