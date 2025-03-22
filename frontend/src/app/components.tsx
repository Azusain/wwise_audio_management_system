import { useState } from "react";
import toast from "react-hot-toast";

const backendUrl: string = "http://localhost:5080";
const apiHeaders = {
  "Content-Type": "application/json",
};

const Header = () => {
  return (
    <div className="navbar bg-base-100">
      <div className="navbar-start"></div>
      <div className="navbar-center"></div>
      <div className="navbar-end">
        <button className="btn btn-ghost btn-circle">
          <svg
            xmlns="http://www.w3.org/2000/svg"
            className="h-5 w-5"
            fill="none"
            viewBox="0 0 24 24"
            stroke="currentColor"
          >
            <path
              strokeLinecap="round"
              strokeLinejoin="round"
              strokeWidth="2"
              d="M21 21l-6-6m2-5a7 7 0 11-14 0 7 7 0 0114 0z"
            />
          </svg>
        </button>
        <button className="btn btn-ghost btn-circle">
          <div className="indicator">
            <svg
              xmlns="http://www.w3.org/2000/svg"
              className="h-5 w-5"
              fill="none"
              viewBox="0 0 24 24"
              stroke="currentColor"
            >
              <path
                strokeLinecap="round"
                strokeLinejoin="round"
                strokeWidth="2"
                d="M15 17h5l-1.405-1.405A2.032 2.032 0 0118 14.158V11a6.002 6.002 0 00-4-5.659V5a2 2 0 10-4 0v.341C7.67 6.165 6 8.388 6 11v3.159c0 .538-.214 1.055-.595 1.436L4 17h5m6 0v1a3 3 0 11-6 0v-1m6 0H9"
              />
            </svg>
            <span className="badge badge-xs badge-primary indicator-item"></span>
          </div>
        </button>
      </div>
    </div>
  );
};

interface SidebarElemInterface {
  name: string;
  setSidebar: React.Dispatch<React.SetStateAction<string>>;
  sidebar: string;
}

const SidebarElement: React.FC<SidebarElemInterface> = (props) => {
  return (
    <li>
      <a
        className={props.sidebar == props.name ? "bg-gray-700 text-white" : ""}
        onClick={() => {
          props.setSidebar(props.name);
        }}
      >
        {props.name}
      </a>
    </li>
  );
};

interface SidebarInterface {
  sidebar: string;
  setSidebar: React.Dispatch<React.SetStateAction<string>>;
}

const Sidebar: React.FC<SidebarInterface> = (props) => {
  const sidebarElems = ["导入", "统计"];

  return (
    <div className="drawer lg:drawer-open">
      <input id="my-drawer-2" type="checkbox" className="drawer-toggle" />
      <div className="drawer-content flex flex-col items-center justify-center">
        {/* Page content here */}
        <label
          htmlFor="my-drawer-2"
          className="btn btn-primary drawer-button lg:hidden"
        >
          Open drawer
        </label>
      </div>
      <div className="drawer-side">
        <label
          htmlFor="my-drawer-2"
          aria-label="close sidebar"
          className="drawer-overlay"
        ></label>
        <ul className="menu bg-base-200 text-base-content min-h-full w-80 p-4">
          {/* Sidebar content here */}
          <div className="font-semibold text-lg p-4">Wwise 管理工具</div>
          <div className="divider"></div>
          {sidebarElems.map((elem, idx) => {
            return (
              <SidebarElement
                key={idx}
                name={elem}
                setSidebar={props.setSidebar}
                sidebar={props.sidebar}
              />
            );
          })}
        </ul>
      </div>
    </div>
  );
};

function extractFileName(filePath: string): string {
  const fileName = filePath.split(/[\\/]/).pop() || "";
  const lastDotIndex = fileName.lastIndexOf(".");
  return lastDotIndex > 0 ? fileName.substring(0, lastDotIndex) : fileName;
}

function convertToWwiseObject(containerType: string): string {
  const mapping: Record<string, string> = {
    "顺序容器 (Sequence Container)": "<Sequence Container>",
    "随机容器 (Random Container)": "<Random Container>",
    "切换容器 (Switch Container)": "<Switch Container>",
    "混合容器 (Blend Container)": "<Blend Container>",
  };

  return mapping[containerType] || "";
}

// TODO: Remote file uploading.
// const FileUploadComponent = () => {
//   const { getRootProps, getInputProps } = useDropzone({
//     onDrop: (acceptedFiles: File[]) => {
//       acceptedFiles.forEach((file: File) => {
//         console.log("文件路径:", file.name);
//       });
//     },
//   });
//   return (
//     <div
//       {...getRootProps()}
//       className="p-6 border-2 border-dashed rounded-lg cursor-pointer text-center"
//     >
//       <input {...getInputProps()} />
//       <p className="text-gray-500">拖拽文件到这里，或点击选择文件</p>
//     </div>
//   );
// };

interface ImportedAudioEntry {
  localFilePath: string;
}
//
// <Sound SFX> or <Sound Voice>
const AudioImportMainContent = () => {
  const [selectedContainerType, setSelectedContainerType] = useState(
    "顺序容器 (Sequence Container)"
  );
  const [parentPath, setParentPath] = useState("");
  const [audioEntries, setAudioEntries] = useState<ImportedAudioEntry[]>();
  const [newContainerName, setNewContainerName] = useState("");
  // <Sound SFX> or <Sound Voice>
  const [importType, setImportType] = useState("<Sound SFX>");
  const [importLang, setImportLang] = useState("English(US)");
  const [userLangs, setUserLangs] = useState<string[]>();

  const getSelectedPath = async () => {
    const resp = await fetch(`${backendUrl}/getSelectedPath`, {
      method: "GET",
      headers: apiHeaders,
    });
    const resp_json = await resp.json();
    if (resp.status != 200) {
      toast.error(JSON.stringify(resp_json));
      return;
    }
    const objects = resp_json["objects"];
    if (objects == null || objects.length == 0) {
      toast.error("failed to get selected path");
      return;
    }
    setParentPath(objects[0]["path"]);
    toast.success(objects[0]["path"]);
  };

  const bringToForeground = async () => {
    await fetch(`${backendUrl}/findwwise`, {
      method: "GET",
      headers: apiHeaders,
    });
  };

  const highlightObject = async (id: string) => {
    await fetch(`${backendUrl}/highlight`, {
      method: "POST",
      headers: apiHeaders,
      body: JSON.stringify({
        id: id,
      }),
    });
  };

  const getGUIDFromPath = async (path: string) => {
    const resp = await fetch(`${backendUrl}/guid`, {
      method: "POST",
      headers: apiHeaders,
      body: JSON.stringify({
        path: path,
      }),
    });
    const resp_json = await resp.json();
    const ret = resp_json["return"];
    if (resp.status != 200 || ret.length == 0) {
      toast.error(`failed to get GUID from path ${path}`);
      return "";
    }
    return ret[0]["id"];
  };

  const getUserLanguages = async () => {
    const resp = await fetch(`${backendUrl}/languages`, {
      method: "GET",
      headers: apiHeaders,
    });
    const resp_json = await resp.json();
    const rawUserLangs = resp_json["return"].map(
      (obj: { name: string }) => obj.name
    );
    setUserLangs(
      rawUserLangs.filter(
        (name: string) => name != "SFX" && name != "External" && name != "Mixed"
      )
    );
  };

  const getSelectedFiles = async () => {
    const resp = await fetch(`${backendUrl}/select`, {
      method: "GET",
      headers: apiHeaders,
    });
    const json_resp = await resp.json();
    const addedFiles = json_resp.files.map((path: string) => ({
      localFilePath: path,
    }));
    setAudioEntries([
      ...(audioEntries || []),
      ...json_resp.files.map((path: string) => ({
        localFilePath: path,
        ...addedFiles,
      })),
    ]);
    toast.success(`成功添加 ${addedFiles.length} 个音频文件`);
  };

  const importAudio = async () => {
    // Check necessary fields.
    if (
      convertToWwiseObject(selectedContainerType) != "" &&
      newContainerName == ""
    ) {
      toast.error("还未设置新容器名称");
      return;
    }
    if (
      audioEntries == null ||
      (audioEntries != null && audioEntries.length == 0)
    ) {
      toast.error("没有添加的音频文件");
      return;
    }
    // Call api.
    const convertedContainerType = convertToWwiseObject(selectedContainerType);
    const newContainerPath =
      convertedContainerType != ""
        ? `\\${convertedContainerType}${newContainerName}`
        : "";
    const resp = await fetch(`${backendUrl}/import`, {
      method: "POST",
      headers: apiHeaders,
      body: JSON.stringify({
        audioFiles: audioEntries?.map((enrty: ImportedAudioEntry) => ({
          audioFile: enrty.localFilePath,
          objectPath: `${parentPath}${newContainerPath}\\${importType}${extractFileName(
            enrty.localFilePath
          )}`,
        })),
        importLang: importType == "<Sound Voice>" ? importLang : "",
      }),
    });
    const json_resp = await resp.json();
    if (resp.status != 200) {
      toast.error(json_resp);
      return;
    }
    toast.success("导入成功");
    // After imnporting audio.
    await bringToForeground();
    const guid = await getGUIDFromPath(
      parentPath + (newContainerName == "" ? "" : `\\${newContainerName}`)
    );
    await highlightObject(guid);
  };

  return (
    <div>
      {/* set parent path */}
      <div>
        <label className="block">设定导入位置</label>
        <div className="flex flex-row">
          <input
            type="text"
            value={parentPath}
            className="input border-black"
            onChange={(e) => {
              setParentPath(e.target.value);
            }}
          />
          <button className="btn btn-outline" onClick={getSelectedPath}>
            使用 Wwise 中选中的路径
          </button>
        </div>
      </div>
      {/* create a new container */}
      <div className="flex flex-row">
        <div>
          <label className="block">创建新容器</label>
          <select
            className="select select-bordered"
            value={selectedContainerType}
            onChange={(e) => {
              setSelectedContainerType(e.target.value);
            }}
          >
            <option>顺序容器 (Sequence Container)</option>
            <option>随机容器 (Random Container)</option>
            <option>切换容器 (Switch Container)</option>
            <option>混合容器 (Blend Container)</option>
            <option>不创建容器 (None)</option>
          </select>
        </div>
        <div
          className={
            selectedContainerType == "不创建容器 (None)" ? "hidden" : ""
          }
        >
          <label className="block">新容器名称</label>
          <input
            type="text"
            className="input border-black"
            onChange={(e) => {
              setNewContainerName(e.target.value);
            }}
          ></input>
        </div>
      </div>
      {/* import type */}
      <div className="flex flex-row">
        <div>
          <label className="block">导入类型</label>
          <select
            className="select select-bordered"
            value={importType.slice(1, -1)}
            onChange={(e) => {
              setImportType(`<${e.target.value}>`);
            }}
            onClick={getUserLanguages}
          >
            <option>{"Sound SFX"}</option>
            <option>{"Sound Voice"}</option>
          </select>
        </div>
        <div className={importType == "<Sound SFX>" ? "hidden" : ""}>
          <label className="block">语言</label>
          <select
            className="select select-bordered"
            value={importLang}
            onChange={(e) => {
              setImportLang(e.target.value);
            }}
          >
            {userLangs?.map((language, idx) => {
              return <option key={idx}>{language}</option>;
            })}
          </select>
        </div>
      </div>
      {/* import button */}
      <button className="btn btn-outline" onClick={importAudio}>
        导入
      </button>
      {/* select files button */}
      <button className="btn btn-outline" onClick={getSelectedFiles}>
        添加文件
      </button>
      {/* choose files */}
      {/* TODO: Remote file upload. */}
      {/* <div>
        <FileUploadComponent />
      </div> */}
      {/* audio files ready for importing */}
      <table className="table">
        <thead>
          <tr>
            <th>编号</th>
            <th>文件路径</th>
            <th>操作</th>
          </tr>
        </thead>
        {
          <tbody>
            {audioEntries &&
              audioEntries.map((entry, idx) => {
                return (
                  <tr key={idx}>
                    <th>{idx}</th>
                    <td>{entry.localFilePath}</td>
                    <td>
                      <button
                        className="btn"
                        onClick={() => {
                          setAudioEntries((prevEntries = []) =>
                            prevEntries.filter((_, index) => index !== idx)
                          );
                        }}
                      >
                        删除
                      </button>
                    </td>
                  </tr>
                );
              })}
          </tbody>
        }
      </table>
    </div>
  );
};

export { Sidebar, SidebarElement, Header, AudioImportMainContent };
export type { SidebarElemInterface };
