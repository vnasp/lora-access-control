import Image from "next/image";
import RefreshButton from "./RefreshButton";
import LogoutButton from "./LogoutButton";

export default function NavBar({ now }: { now: string }) {
  return (
    <nav className="relative z-10 flex justify-center px-6 py-3 text-text-primary">
      <div className="mx-auto flex w-full max-w-7xl items-center justify-between">
        <div className="flex flex-row items-center gap-4">
          <Image
            src="/logo.png"
            alt="LoRa Access Control"
            width={75}
            height={75}
            className="aspect-auto"
          />
          <div className="flex flex-col">
            <h1 className="text-2xl font-bold">LoRa ACCESS CONTROL</h1>
            <p>Acceso comunitario • Red LoRa</p>
          </div>
        </div>
        <div className="flex items-center gap-3">
          <div className="text-sm">
            <div className="flex items-center gap-1.5 font-semibold">
              <span className="inline-block h-2 w-2 rounded-full bg-green-500" />
              Sistema en línea
            </div>
            <div>Última actualización: {now}</div>
          </div>
          <RefreshButton />
          <LogoutButton />
        </div>
      </div>
    </nav>
  );
}
