"use client";
import React, { useState } from "react";
import { AudioImportMainContent, Header, Sidebar } from "./components";
import { Toaster } from "react-hot-toast";

export default function Home() {
  const [sidebar, setSidebar] = useState("导入");

  return (
    <div className="flex flex-row">
      <div>
        <Toaster position="bottom-right" />
      </div>

      <div className="w-1/4">
        <Sidebar setSidebar={setSidebar} sidebar={sidebar} />
      </div>
      <div className="w-3/4">
        <Header />
        {/* main content */}
        {sidebar == "导入" && <AudioImportMainContent />}
        {/* {sidebar == "统计" && <SettingsMainContent />} */}
      </div>
    </div>
  );
}

// TODO:
// static resources should be download beforehand.
// create a folder for api codes.
