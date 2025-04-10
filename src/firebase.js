// src/firebase.js
import { initializeApp } from "firebase/app";
import { getDatabase } from "firebase/database";
import { getAuth } from "firebase/auth";

const firebaseConfig = {
  apiKey: "AIzaSyCuf2LskYlhTVFPL1YcgB263mPQfTIVVAs",
  authDomain: "fingerprintscanner-1b2d6.firebaseapp.com",
  databaseURL: "https://fingerprintscanner-1b2d6-default-rtdb.firebaseio.com",
  projectId: "fingerprintscanner-1b2d6",
  storageBucket: "fingerprintscanner-1b2d6.firebasestorage.app",
  messagingSenderId: "374751172278",
  appId: "1:374751172278:web:260dda8a3eaa38dcf17817",
  measurementId: "G-BGDE88F0X9"
};

const app = initializeApp(firebaseConfig);
const db = getDatabase(app);
const auth = getAuth(app);

export { db, auth };
