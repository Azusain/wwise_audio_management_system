import { useState } from "react";
import { useDropzone } from "react-dropzone";

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

// interface AddIpButtonInterface {
//   getIpList: () => void;
// }

// const AddIpButton: React.FC<AddIpButtonInterface> = (props) => {
//   const [newIp, setNewIp] = useState("");
//   return (
//     <div>
//       <button
//         className="btn"
//         onClick={() => {
//           const elem: HTMLElement | null =
//             document.getElementById("add_ip_modal");
//           if (elem) {
//             (elem as HTMLDialogElement).showModal();
//           }
//         }}
//       >
//         Add IP
//       </button>
//       <dialog id="add_ip_modal" className="modal">
//         <div className="modal-box">
//           <h3 className="font-bold text-lg pb-4">New IP Address</h3>
//           <label className="form-control w-full max-w-xs">
//             <input
//               type="text"
//               placeholder="IPv4 Address"
//               className="input input-bordered w-full max-w-xs"
//               onChange={(e) => {
//                 setNewIp(e.target.value);
//               }}
//             />
//           </label>{" "}
//           <div className="modal-action">
//             <form method="dialog" className="flex flex-row gap-4">
//               <button
//                 className="btn"
//                 onClick={() => {
//                   const addNewIp = async () => {
//                     await fetch(`${backendUrl}/config/iplist:update`, {
//                       method: "POST",
//                       headers: apiHeaders,
//                       body: JSON.stringify({
//                         action: "ADD",
//                         ip_list: [newIp],
//                       }),
//                     });
//                   };
//                   addNewIp();
//                   props.getIpList();
//                 }}
//               >
//                 Add
//               </button>
//               <button className="btn">Cancel</button>
//             </form>
//           </div>
//         </div>
//       </dialog>
//     </div>
//   );
// };

// interface FirewallTableInterface {
//   getIpList: () => void;
//   setIpList: Dispatch<SetStateAction<string[] | undefined>>;
//   ipList: string[];
// }

// const FirewallTable: React.FC<FirewallTableInterface> = (props) => {
//   useEffect(() => {
//     props.getIpList();
//   }, [props]);

//   return (
//     <div className="overflow-x-auto">
//       <table className="table">
//         <thead>
//           <tr>
//             <th>Index</th>
//             <th>IP</th>
//             <th>Modify</th>
//           </tr>
//         </thead>
//         {props.ipList ? (
//           <tbody>
//             {props.ipList.map((ip, idx) => {
//               return (
//                 <tr key={idx}>
//                   <th>{idx}</th>
//                   <td>{ip}</td>
//                   <td>
//                     <button
//                       className="btn"
//                       onClick={() => {
//                         const deleteIp = async () => {
//                           await fetch(`${backendUrl}/config/iplist:update`, {
//                             method: "POST",
//                             headers: apiHeaders,
//                             body: JSON.stringify({
//                               action: "REMOVE",
//                               ip_list: [props.ipList[idx]],
//                             }),
//                           });
//                         };
//                         deleteIp();
//                       }}
//                     >
//                       Delete
//                     </button>
//                   </td>
//                 </tr>
//               );
//             })}
//           </tbody>
//         ) : (
//           <></>
//         )}
//       </table>
//     </div>
//   );
// };

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

// const FirewallMainContent = () => {
//   const [ipList, setipList] = useState<string[]>();
//   const getIpList = async () => {
//     const response = await fetch(`${backendUrl}/config/iplist`, {
//       method: "GET",
//       headers: apiHeaders,
//     });
//     const result: FirewallTableInterface = await response.json();
//     setipList(result.ipList);
//   };
//   return (
//     <>
//       <AddIpButton getIpList={getIpList} />
//       <FirewallTable
//         getIpList={getIpList}
//         setIpList={setipList}
//         ipList={ipList ?? []}
//       />
//     </>
//   );
// };

// interface ToggleSwitchInterface {
//   optionName: string;
//   on: boolean;
//   getConfig: () => void;
// }

// interface UpdateConfigRequest {
//   path: string[];
//   httpCompression: boolean;
// }

// interface GetConfigResponse {
//   httpCompression: boolean;
// }

// const updateConfig = async (request: UpdateConfigRequest) => {
//   await fetch(`${backendUrl}/config:update`, {
//     method: "POST",
//     headers: apiHeaders,
//     body: JSON.stringify(request),
//   });
// };

// const ToggleSwitch: React.FC<ToggleSwitchInterface> = (props) => {
//   return (
//     <div className="flex flex-row gap-4 justify-between w-1/4">
//       <label>{props.optionName}</label>
//       <input
//         type="checkbox"
//         className="toggle"
//         checked={props.on ?? false}
//         onChange={(e) => {
//           updateConfig({
//             path: ["http_compression"],
//             httpCompression: e.target.checked,
//           }).then(() => {
//             props.getConfig();
//           });
//         }}
//       />
//     </div>
//   );
// };

// const SettingsMainContent = () => {
//   const [httpCompression, setHttpCompression] = useState(false);
//   const getConfig = async () => {
//     const response = await fetch(`${backendUrl}/config`, {
//       method: "GET",
//       headers: apiHeaders,
//     });
//     const result: GetConfigResponse = await response.json();
//     setHttpCompression(result.httpCompression);
//   };
//   useEffect(() => {
//     getConfig();
//   }, []);

//   return (
//     <div className="flex flex-col gap-4">
//       <ToggleSwitch
//         getConfig={getConfig}
//         optionName="Enable HTTP Compression"
//         on={httpCompression}
//       />
//     </div>
//   );
// };

const FileUploadComponent = () => {
  const { getRootProps, getInputProps } = useDropzone({
    onDrop: (acceptedFiles: File[]) => {
      acceptedFiles.forEach((file: File) => {
        console.log("文件路径:", file.name);
      });
    },
  });
  return (
    <div
      {...getRootProps()}
      className="p-6 border-2 border-dashed rounded-lg cursor-pointer text-center"
    >
      <input {...getInputProps()} />
      <p className="text-gray-500">拖拽文件到这里，或点击选择文件</p>
    </div>
  );
};

interface ImportedAudioEntry {
  name: string;
  type: string;
}

const AudioImportMainContent = () => {
  const [selectedOptions, setSelectedOptions] = useState(
    "顺序容器 (Sequence Container)"
  );
  const [parentPath, setParentPath] = useState("");
  const [audioEntries, setAudioEntries] = useState<ImportedAudioEntry[]>();

  const getSelectedFiles = async () => {
    const resp = await fetch(`http://localhost:5080/select`, {
      method: "GET",
      headers: apiHeaders,
    });
    const json_resp = await resp.json();
    console.log(json_resp);
  };

  return (
    <div>
      {/* set parent path */}
      <label className="block">设定导入位置</label>
      <input
        type="text"
        className="input border-black"
        onChange={(e) => {
          setParentPath(e.target.value);
        }}
      ></input>
      {/* select target container */}
      <div>
        <label className="block">选择目标容器</label>
        <select
          className="select select-bordered"
          value={selectedOptions}
          onChange={(e) => {
            setSelectedOptions(e.target.value);
          }}
        >
          <option>顺序容器 (Sequence Container)</option>
          <option>随机容器 (Random Container)</option>
          <option>切换容器 (Switch Container)</option>
          <option>融合容器 (Blend Container)</option>
          <option>不创建容器 (None)</option>
        </select>
      </div>
      {/* import button */}
      <button
        className="btn btn-outline"
        onClick={() => {
          console.log(selectedOptions);
          console.log(parentPath);
        }}
      >
        导入
      </button>
      {/* select files button */}
      <button className="btn btn-outline" onClick={getSelectedFiles}>
        选择文件
      </button>

      {/* choose files */}
      <div>
        <FileUploadComponent />
      </div>
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
                    <td>{entry.name}</td>
                    <td>
                      <button className="btn">Delete</button>
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
