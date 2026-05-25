import { supabase } from "@/lib/supabase";

export const revalidate = 30; // refresca cada 30 segundos

type LogRow = {
  id: string;
  timestamp: string;
  code: string;
  parcel_id: number | null;
};

function toChileTime(utc: string) {
  return new Intl.DateTimeFormat("es-CL", {
    timeZone: "America/Santiago",
    dateStyle: "short",
    timeStyle: "medium",
  }).format(new Date(utc));
}

export default async function Home() {
  const { data: logs } = await supabase
    .from("lora_access_logs")
    .select("id, timestamp, code, parcel_id")
    .order("timestamp", { ascending: false })
    .limit(50);

  return (
    <div className="min-h-screen bg-zinc-50 p-8">
      <div className="mx-auto max-w-3xl">
        <h1 className="mb-6 text-2xl font-semibold text-zinc-800">
          LoRa Gateway — Accesos
        </h1>

        {!logs || logs.length === 0 ? (
          <p className="text-zinc-500">Sin registros todavía.</p>
        ) : (
          <div className="overflow-hidden rounded-xl border border-zinc-200 bg-white shadow-sm">
            <table className="w-full text-sm">
              <thead className="bg-zinc-100 text-left text-zinc-500">
                <tr>
                  <th className="px-4 py-3 font-medium">Fecha y hora (Chile)</th>
                  <th className="px-4 py-3 font-medium">Parcela</th>
                  <th className="px-4 py-3 font-medium">Código</th>
                </tr>
              </thead>
              <tbody className="divide-y divide-zinc-100">
                {(logs as LogRow[]).map((row) => (
                  <tr key={row.id} className="hover:bg-zinc-50">
                    <td className="px-4 py-3 text-zinc-700">
                      {toChileTime(row.timestamp)}
                    </td>
                    <td className="px-4 py-3 font-medium text-zinc-800">
                      {row.parcel_id ?? "—"}
                    </td>
                    <td className="px-4 py-3 font-mono text-zinc-600">
                      {row.code}
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        )}
      </div>
    </div>
  );
}
