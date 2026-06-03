"use client";

import { useState, useEffect } from "react";
import { LogIn } from "lucide-react";
import type { LogRow } from "@/app/types";
import NavBar from "./NavBar";
import Dashboard from "./Dashboard";
import Footer from "./Footer";
import Image from "next/image";

type Props = {
  allCodes: Record<number, string>;
  allLogs: LogRow[];
  now: string;
};

const STORAGE_KEY = "lora_auth";

function todayChile() {
  return new Intl.DateTimeFormat("en-CA", {
    timeZone: "America/Santiago",
  }).format(new Date());
}

function toChileDateYMD(utc: string) {
  return new Intl.DateTimeFormat("en-CA", {
    timeZone: "America/Santiago",
  }).format(new Date(utc));
}

function toChileHHMM(utc: string) {
  return new Intl.DateTimeFormat("es-CL", {
    timeZone: "America/Santiago",
    hour: "2-digit",
    minute: "2-digit",
    hour12: false,
  }).format(new Date(utc));
}

export default function LoginGate({ allCodes, allLogs, now }: Props) {
  const [parcelId, setParcelId] = useState<number | null>(null);
  const [mounted, setMounted] = useState(false);
  const [formParcel, setFormParcel] = useState("1");
  const [rut, setRut] = useState("");
  const [error, setError] = useState("");
  const [loading, setLoading] = useState(false);

  useEffect(() => {
    let id: number | null = null;
    const stored = localStorage.getItem(STORAGE_KEY);
    if (stored) {
      try {
        const { parcel } = JSON.parse(stored);
        id = Number(parcel);
      } catch {
        localStorage.removeItem(STORAGE_KEY);
      }
    }
    // Reading localStorage must happen client-side after mount — batching both
    // updates together avoids a second render cycle.
    // eslint-disable-next-line react-hooks/set-state-in-effect
    setParcelId(id);
    setMounted(true);
  }, []);

  async function handleSubmit(e: React.FormEvent) {
    e.preventDefault();
    if (!rut.trim()) {
      setError("Ingresa tu RUT");
      return;
    }
    setLoading(true);
    setError("");
    try {
      const res = await fetch("/api/auth", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({
          parcel_id: Number(formParcel),
          rut: rut.trim(),
        }),
      });
      const data = await res.json();
      if (!res.ok || !data.ok) {
        setError(data.error ?? "Parcela o RUT incorrecto");
        return;
      }
      const parcel = Number(formParcel);
      localStorage.setItem(
        STORAGE_KEY,
        JSON.stringify({ parcel, rut: rut.trim() }),
      );
      setParcelId(parcel);
    } catch {
      setError("Error de conexión. Intenta de nuevo.");
    } finally {
      setLoading(false);
    }
  }

  // Evitar flash de contenido antes de leer localStorage
  if (!mounted) return null;

  if (parcelId === null) {
    return (
      <div className="fixed inset-0 z-50 flex items-center justify-center bg-overlay backdrop-blur-sm">
        <form
          onSubmit={handleSubmit}
          className="w-full max-w-sm rounded-2xl bg-surface p-8 shadow-2xl"
        >
          <div className="mb-6 flex flex-col items-center justify-center">
            <Image
              src="/logo.png"
              alt="LoRa Access Control"
              width={75}
              height={75}
              className="aspect-auto"
            />
            <h2 className="text-xl font-bold text-text-primary">
              Acceso al sistema
            </h2>
            <p className="mt-1 text-sm text-text-muted">
              LoRa Access Control · Comunidad
            </p>
          </div>

          <div className="space-y-4">
            <div>
              <label className="mb-1.5 block text-xs font-semibold uppercase tracking-wide text-text-secondary">
                Número de parcela
              </label>
              <select
                value={formParcel}
                onChange={(e) => setFormParcel(e.target.value)}
                className="w-full rounded-xl border border-border bg-surface-muted px-4 py-2.5 text-sm text-text-primary"
              >
                {Array.from({ length: 15 }, (_, i) => (
                  <option key={i + 1} value={String(i + 1)}>
                    Parcela {String(i + 1).padStart(2, "0")}
                  </option>
                ))}
              </select>
            </div>

            <div>
              <label className="mb-1.5 block text-xs font-semibold uppercase tracking-wide text-text-secondary">
                RUT
              </label>
              <input
                type="text"
                value={rut}
                onChange={(e) => {
                  setRut(e.target.value);
                  setError("");
                }}
                placeholder="12.345.678-9"
                className="w-full rounded-xl border border-border bg-surface-muted px-4 py-2.5 text-sm text-text-primary placeholder:text-text-muted"
              />
              {error && <p className="mt-1 text-xs text-error">{error}</p>}
            </div>
          </div>

          <button
            type="submit"
            disabled={loading}
            className="mt-6 flex w-full items-center justify-center gap-2 rounded-xl bg-primary py-2.5 text-sm font-semibold text-white transition hover:bg-primary-dark disabled:opacity-60"
          >
            <LogIn className="h-4 w-4" />
            {loading ? "Verificando..." : "Ingresar"}
          </button>
        </form>
      </div>
    );
  }

  // Stats filtrados por parcela del usuario
  const today = todayChile();
  const todayLogs = allLogs.filter(
    (l) => toChileDateYMD(l.timestamp) === today,
  );
  const entries = todayLogs.filter((l) => l.event_type === "ENTRY").length;
  const exits = todayLogs.filter((l) => l.event_type === "EXIT").length;
  const activeParcels = new Set(todayLogs.map((l) => l.parcel_id)).size;
  const lastEvent = todayLogs[0] ? toChileHHMM(todayLogs[0].timestamp) : "—";

  return (
    <>
      <NavBar now={now} />
      <Dashboard
        parcelId={parcelId}
        dailyCode={allCodes[parcelId] ?? null}
        allLogs={allLogs}
        entries={entries}
        exits={exits}
        activeParcels={activeParcels}
        lastEvent={lastEvent}
      />
      <Footer />
    </>
  );
}
