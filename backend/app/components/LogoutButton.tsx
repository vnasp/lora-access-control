"use client";

import { LogOut } from "lucide-react";

export default function LogoutButton() {
  function handleLogout() {
    localStorage.removeItem("lora_auth");
    window.location.reload();
  }

  return (
    <button
      onClick={handleLogout}
      title="Cerrar sesión"
      className="rounded p-1.5 transition hover:bg-white/10"
    >
      <LogOut className="h-5 w-5" />
    </button>
  );
}
