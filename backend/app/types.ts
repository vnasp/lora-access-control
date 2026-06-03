export type LogRow = {
  id: string;
  timestamp: string;
  code: string;
  parcel_id: number | null;
  event_type: "ENTRY" | "EXIT" | null;
};
