import { supabase } from "@/lib/supabase";
import LoginGate from "./components/LoginGate";
import type { LogRow } from "./types";

export const revalidate = 30;

function todayChile() {
  return new Intl.DateTimeFormat("en-CA", {
    timeZone: "America/Santiago",
  }).format(new Date());
}

function toChileFullTime(utc: string) {
  return new Intl.DateTimeFormat("es-CL", {
    timeZone: "America/Santiago",
    hour: "2-digit",
    minute: "2-digit",
    second: "2-digit",
    hour12: false,
  }).format(new Date(utc));
}

export default async function Home() {
  const today = todayChile();

  const [{ data: codesData }, { data: logs }] = await Promise.all([
    supabase
      .from("lora_daily_codes")
      .select("parcel_id, code")
      .eq("date", today),
    supabase
      .from("lora_access_logs")
      .select("id, timestamp, code, parcel_id, event_type")
      .order("timestamp", { ascending: false })
      .limit(50),
  ]);

  const allCodes = Object.fromEntries(
    (codesData ?? []).map((r) => [r.parcel_id, r.code]),
  ) as Record<number, string>;

  const rawLogs = (logs ?? []) as LogRow[];

  // Mask the code only for today's ENTRY records with no corresponding EXIT yet.
  // Past-day codes are already expired so there's no risk showing them.
  const todayExitCodes = new Set(
    rawLogs
      .filter((l) => l.event_type === "EXIT" && l.timestamp.slice(0, 10) === today)
      .map((l) => l.code),
  );
  const allLogs = rawLogs.map((l) =>
    l.event_type === "ENTRY" &&
    l.timestamp.slice(0, 10) === today &&
    !todayExitCodes.has(l.code)
      ? { ...l, code: "••••••" }
      : l,
  );

  const now = toChileFullTime(new Date().toISOString());

  return (
    <div className="relative flex min-h-screen flex-col bg-background pt-4">
      <div className="absolute inset-x-0 top-0 h-72 bg-[url('/header.png')] bg-cover bg-[50%_30%] bg-no-repeat" />
      <LoginGate allCodes={allCodes} allLogs={allLogs} now={now} />
    </div>
  );
}
